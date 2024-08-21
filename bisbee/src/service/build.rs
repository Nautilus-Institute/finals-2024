fn main() {
    println!("cargo:rustc-link-lib=static=kvdb");
    println!("cargo:rustc-link-search=native=./");
}
