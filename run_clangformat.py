Import("env")

import subprocess
import os
import sys

def check_format_callback(*arg, **kwargs):
    return formatting_callback(arg, kwargs)

def apply_format_callback(*arg, **kwargs):
    print("Formatting Source Code")
    return formatting_callback(arg, kwargs, apply=True)

def formatting_callback(*arg, **kwargs):
    subprocess.check_call([sys.executable, "-m", "pip", "install", "clang-format"])

    folder_list = [env.get("PROJECT_INCLUDE_DIR"), env.get("PROJECT_SRC_DIR")]
    for item in env.get("__PIO_LIB_BUILDERS"):
        folder_list.append(item.path)

    project_dir = env.get("PROJECT_DIR")
    rel_folder_list = [os.path.relpath(i, project_dir) for i in folder_list]
    folders = [d for d in rel_folder_list if not d.startswith('.')]

    file_list = []
    for folder in folders:
        for root, dirs, files in os.walk(folder, topdown=True):
            dirs[:] = [d for d in dirs if not d.startswith('.')]
            files = [f for f in files if not f[0] == '.']

            for file in files:
                if file.endswith( ('.c','.cpp', '.h', '.hpp', '.cc', '.cxx', '.hxx','.hh') ):
                    file_list.append(os.path.join(root, file))

    # for file in file_list:
    if env.Execute("clang-format --Werror" + (" --dry-run " if not kwargs.get("apply", False) else " ") + "-i " + " ".join(f'"{f}"' for f in file_list)):
        env.Exit(1)

env.AddCustomTarget(
    "format",
    None,
    apply_format_callback,
    title="clang-format",
    description="Run Source Code Formatting")

env.AddCustomTarget(
    "check-format",
    None,
    check_format_callback,
    title="clang-format",
    description="Check Source Code Formatting")
