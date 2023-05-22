import os
import subprocess

# Get current directory
current_dir = os.path.dirname(os.path.realpath(__file__))

# Specify the directory to format code in
dir_path = ["Core", 
            "Middlewares/ST/STM32_USB_Device_Library/Class/AUDIO",
            "USB_DEVICE"]

# Specify the path to the clang-format executable
clang_format_path = "clang-format"

# Specify the path to the stylesheet to use
style_path = ".clang-format"


# Find all .c and .cpp files in the specified directory
files_to_format = []
for d in dir_path:
    for root, dirs, files in os.walk(d):
        root = os.path.join(current_dir, root)
        for file in files:
            if file.endswith(".c") or file.endswith(".cpp") or file.endswith(".h"):
                files_to_format.append(os.path.join(root, file))

# Run clang-format on each file with the specified stylesheet
for file_path in files_to_format:
    subprocess.run([clang_format_path, "-i", "-style=file", "-assume-filename=" + file_path, "-fallback-style=none", file_path])

print("Done formatting!")