import os
import shutil
import zipfile
import tokenize
import io
import re
from pathlib import Path
from tempfile import TemporaryDirectory

# Hardcoded list of file paths
file_paths = [
    "stateconverter.py", "simulatoraiparallel.py", "LLM.py", "game.py","state.py","visualizer.py"
]
zip_path = "public.zip"


os.system("rm "+zip_path)
file_paths = ["game/"+f for f in file_paths]

# Hardcoded strings to remove text in between
start_marker = "###START_REMOVE"
end_marker = "###END_REMOVE"


def remove_comments(source_code):
    # Pattern to match strings or comments
    pattern = r"""
        (\".*?\"|\'.*?\')  # Match double or single quoted strings
        |                 # OR
        (\#.*?$)          # Match comments
    """
    # Function to decide what to replace with
    def replace(match):
        if match.group(2):  # If the second group (comment) is matched
            return ''       # Replace it with nothing
        return match.group(1)  # Otherwise, return the matched string as is

    # Replace based on the pattern
    cleaned_code = re.sub(pattern, replace, source_code, flags=re.MULTILINE | re.VERBOSE)
    return cleaned_code


def remove_text_between(text, start, end):
    """Remove text between two specified markers."""
    pattern = re.escape(start) + '.*?' + re.escape(end)
    return re.sub(pattern, '', text, flags=re.DOTALL)

def process_file(file_path, temp_dir):
    """Process a single file to remove comments and specific text."""
    with open(file_path, 'r', encoding='utf-8') as file:
        content = file.read()

    content = remove_text_between(content, start_marker, end_marker)
    content = remove_comments(content)

    tl = []
    for l in content.split("\n"):
        if not l.strip() == "":
            tl.append(l)
    content = "\n".join(tl)
    while "\n\n" in content:
        content = content.replace("\n\n","\n")

    # Write the processed content to a new file in the temporary directory
    temp_file_path = os.path.join(temp_dir, os.path.basename(file_path))
    with open(temp_file_path, 'w', encoding='utf-8') as file:
        file.write(content)

def create_zip_from_folder(folder_path, zip_file_path):
    """Create a zip file from all the files in a folder."""
    with zipfile.ZipFile(zip_file_path, 'w') as zipf:
        for file in Path(folder_path).iterdir():
            zipf.write(file, arcname=file.name)

def main():
    # Create a temporary directory
    with TemporaryDirectory() as temp_dir:
        # Process each file in the list
        for file_path in file_paths:
            process_file(file_path, temp_dir)
        # Create a zip file with all processed files
        create_zip_from_folder(temp_dir, zip_path)

# Run the script
if __name__ == "__main__":
    main()
