#!/bin/bash

# Remove and recreate the .vscode directory
rm -rf .vscode
mkdir .vscode

# Create launch.json
cat <<EOL > .vscode/launch.json
{
  "version": "0.2.0",
  "configurations": [
    {
      "type": "lldb",
      "request": "launch",
      "name": "Debug",
      "program": "\${workspaceFolder}/\${fileBasenameNoExtension}",
      "args": [],
      "cwd": "\${workspaceFolder}",
      "preLaunchTask": "C/C++: gcc build active file"
    }
  ]
}
EOL

# Create tasks.json
cat <<EOL > .vscode/tasks.json
{
  "tasks": [
    {
      "type": "cppbuild",
      "label": "C/C++: gcc build active file",
      "command": "/usr/bin/gcc",
      "args": [
        "-g",
        "\${file}",
        "-o",
        "\${fileDirname}/\${fileBasenameNoExtension}"
      ],
      "options": {
        "cwd": "\${workspaceFolder}"
      },
      "problemMatcher": [
        "\$gcc"
      ],
      "group": {
        "kind": "build",
        "isDefault": true
      },
      "detail": "Generated task by Debugger"
    }
  ],
  "version": "2.0.0"
}
EOL

echo "VS Code debugging configuration created successfully."
