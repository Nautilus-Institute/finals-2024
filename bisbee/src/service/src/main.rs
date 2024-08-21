extern crate libc;
extern crate base64;
extern crate rand;
extern crate md5;

use tokio::net::TcpListener;
use tokio::io::{BufReader, AsyncBufReadExt, AsyncWriteExt};
use async_std::fs;
use std::sync::{Arc, Mutex};
use std::collections::HashMap;
use libc::{c_char, c_void, size_t, ssize_t};
use md5::{Md5, Digest};
use std::io::{self, Write};

extern {
    fn initialize();
    fn store_data(key: *const c_char, data: *const c_void, size: size_t) -> i8;
    fn load_data(key: *const c_char, buffer: *mut c_void, buffer_size: size_t) -> ssize_t;
    fn clear_db();
}

async fn handle_client(
    users: Arc<Mutex<HashMap<String, String>>>,
    blocks: Arc<Mutex<HashMap<String, (String, u32)>>>,
    mut socket: BufReader<tokio::net::TcpStream>,
)
{
    let mut logged_in: bool = false;
    let mut logged_in_user = "".to_string();

    loop {
        let mut request = String::new();

        // read until a newline character is found
        socket.read_line(&mut request).await.unwrap();
        if request.len() == 0 {
            // no more request
            break;
        }

        let command: Vec<&str> = request.trim().split_whitespace().collect();

        match command.get(0) {
            Some(&"REGISTER") => {
                let username = command.get(1).unwrap_or(&"").to_string();
                let password = command.get(2).unwrap_or(&"").to_string();

                // Vuln: We only check the first 32 characters of the user name to ensure they are
                // alphanumeric

                // username must be at most 32 characters long
                if username.len() > 64 {
                    socket.write_all(b"Your user name too long. Try using fewer than 32 characters.\n").await.unwrap();
                    continue;
                }
                let truncated_username = &username[0..32];
                // username must be entirely alphanumeric
                if !truncated_username.chars().all(|c| c.is_alphanumeric()) {
                    socket.write_all(b"Username must be alphanumeric\n").await.unwrap();
                    continue;
                }

                // Simple registration check
                if users.lock().unwrap().contains_key(&username) {
                    socket.write_all(b"Username already exists\n").await.unwrap();
                } else {
                    let mut hasher = Md5::new();
                    hasher.update(password.as_bytes());
                    let password_digest = hasher.finalize();
                    let password_md5 = format!("{:x}", password_digest);
                    users.lock().unwrap().insert(username, password_md5);
                    socket.write_all(b"Registration successful\n").await.unwrap();
                }
            }
            Some(&"LOGIN") => {
                let username = command.get(1).unwrap_or(&"").to_string();
                let password = command.get(2).unwrap_or(&"").to_string();
                let mut hasher = Md5::new();
                hasher.update(password.as_bytes());
                let password_digest = hasher.finalize();
                let password_md5 = format!("{:x}", password_digest);

                // Simple login check
                if users.lock().unwrap().contains_key(&username) && users.lock().unwrap().get(&username) == Some(&password_md5) {
                    socket.write_all(b"Login successful\n").await.unwrap();
                    logged_in = true;
                    logged_in_user = username.clone();
                } else {
                    socket.write_all(b"Login failed\n").await.unwrap();
                    logged_in = false;
                }
            }
            Some(&"GET_BLOCK") => {
                if !logged_in {
                    socket.write_all(b"Please login first\n").await.unwrap();
                    continue;
                }

                let block_id = command.get(1).unwrap_or(&"").parse::<u32>().unwrap().to_string();

                // fallback
                if blocks.lock().unwrap().get(&block_id) == None {
                    socket.write_all(b"The specified block ID is not found\n").await.unwrap();
                    continue;
                }
                let pair = blocks.lock().unwrap().get(&block_id).unwrap().to_owned();
                let (block_name, _) = pair.clone();

                let block_path = format!("{}/{}\x00", logged_in_user, block_name);

                // load data from the database
                unsafe {
                    let mut buffer = vec![0; pair.1 as usize];
                    let size = load_data(block_path.as_ptr() as *const c_char, buffer.as_mut_ptr() as *mut c_void, pair.1 as size_t);
                    if size > 0 {
                        socket.write_all(base64::encode(&buffer[0..size as usize]).as_bytes()).await.unwrap();
                        socket.write_all("\n".as_bytes()).await.unwrap();
                        continue;
                    }
                }

                // Vuln 2: fallback to file system - arbitrary file read
                let block_path = format!("{}/{}", logged_in_user, block_name);
                // io::stdout().write_all(block_path.as_bytes()).unwrap();
                // io::stdout().write_all(b"\n").unwrap();
                match fs::read(block_path).await {
                    Ok(contents) => {
                        let encoded_content = base64::encode(&contents);
                        socket.write_all(encoded_content.as_bytes()).await.unwrap();
                        socket.write_all("\n".as_bytes()).await.unwrap();
                    }
                    Err(_) => {
                        socket.write_all(b"Block not found\n").await.unwrap();
                    }
                }
            }
            Some(&"STORE_BLOCK") => {
                if !logged_in {
                    socket.write_all(b"Please login first\n").await.unwrap();
                    continue;
                }

                let block_name = command.get(1).unwrap_or(&"");
                let block_size = command.get(2).unwrap_or(&"").parse::<u32>().unwrap();

                // ensure block_name only contains alphanumeric characters
                if block_name.chars().any(|c| !c.is_alphanumeric()) {
                    socket.write_all(b"Invalid block name\n").await.unwrap();
                    continue;
                }
                
                // generate a random block ID, store the mapping between block name and block ID in
                // blocks, and then send the block ID to the client
                let block_id = rand::random::<u32>();
                let pair = (block_name.to_string(), block_size);
                blocks.lock().unwrap().insert(block_id.to_string(), pair);
                socket.write_all(format!("Block name: {}, block ID: {}\n", block_name, block_id).as_bytes()).await.unwrap();
            }
            Some(&"SEND_DATA") => {
                if !logged_in {
                    socket.write_all(b"Please login first\n").await.unwrap();
                    continue;
                }

                let block_id = command.get(1).unwrap_or(&"").parse::<u32>().unwrap().to_string();
                let data = command.get(2).unwrap_or(&"");

                // get block name and block size from blocks
                if blocks.lock().unwrap().get(&block_id) == None {
                    socket.write_all(b"Block not found\n").await.unwrap();
                    continue;
                }
                let (block_name, block_size) = blocks.lock().unwrap().get(&block_id).unwrap().to_owned();
                let block_name = block_name.clone();

                // base64-decode data
                // catch any exceptions
                let decoded_data = base64::decode(data).unwrap();

                let block_path = format!("{}/{}\x00", logged_in_user, block_name);

                // store data to the database
                let mut r: i8 = 0;
                unsafe {
                    // Vuln-A: You can store more than the decoded string contains...
                    r = store_data(block_path.as_ptr() as *const c_char, decoded_data.as_ptr() as *const c_void, block_size as size_t);
                }
                r = r & 0x7f;

                // io::stdout().write_all(format!("store_data: {}\n", r).as_bytes()).unwrap();
                if r == 0 {
                    // just in case, we create a directory on the file system using the user name
                    fs::create_dir_all(logged_in_user.clone()).await.unwrap();
                    // write the data in a file
                    let block_file_path = format!("{}/{}", logged_in_user, block_name);
                    fs::write(block_file_path, decoded_data).await.unwrap();
                }

                socket.write_all(b"Block saved\n").await.unwrap();
            }
            Some(&"MINE") => {
                if !logged_in {
                    socket.write_all(b"Please login first\n").await.unwrap();
                    continue;
                }

                socket.write_all(b"Not implemented yet\n").await.unwrap();
            }
            Some(&"LOGOUT") => {
                // let username = command.get(1).unwrap_or(&"");
                // users.lock().unwrap().remove(username);
                logged_in = false;
                socket.write_all(b"Logged out\n").await.unwrap();

                unsafe {
                    clear_db();
                }
            }
            _ => {
                socket.write_all(b"Invalid command\n").await.unwrap();
            }
        }
    }
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>>
{
    let listener = TcpListener::bind("0.0.0.0:61550").await?;
    let users = Arc::new(Mutex::new(HashMap::new()));
    let blocks = Arc::new(Mutex::new(HashMap::new()));

    unsafe {
        initialize();
    }

    loop {
        let (socket, _) = listener.accept().await?;
        let users = users.clone();
        let blocks = blocks.clone();

        let socket = BufReader::new(socket);

        tokio::spawn(async move {
            handle_client(users, blocks, socket).await;
        });
    }
}
