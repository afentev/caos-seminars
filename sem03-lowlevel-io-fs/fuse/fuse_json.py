#!/usr/bin/env python3

import os
import sys
import json
import errno
from fuse import FUSE, FuseOSError, Operations

class JsonFs(Operations):
    def __init__(self, json_data):
        self.json_data = json_data
        self.files = {}
        self.dirs = {'/': []}
        self._build_tree('/', json_data)

    def _build_tree(self, path, data):
        if isinstance(data, dict):
            self.dirs[path] = list(data.keys())
            for key, value in data.items():
                new_path = os.path.join(path, key)
                if isinstance(value, (dict, list)):
                    self.dirs[new_path] = []
                    self._build_tree(new_path, value)
                else:
                    self.files[new_path] = str(value).encode('utf-8')
        elif isinstance(data, list):
            self.dirs[path] = [str(i) for i in range(len(data))]
            for i, item in enumerate(data):
                new_path = os.path.join(path, str(i))
                if isinstance(item, (dict, list)):
                    self.dirs[new_path] = []
                    self._build_tree(new_path, item)
                else:
                    self.files[new_path] = str(item).encode('utf-8')

    def getattr(self, path, fh=None):
        if path in self.files:
            return {
                'st_mode': 0o100644 | 0o444,  # -r--r--r--
                'st_nlink': 1,
                'st_size': len(self.files[path])
            }
        elif path in self.dirs:
            return {
                'st_mode': 0o040755 | 0o555,  # dr-xr-xr-x
                'st_nlink': 2
            }
        else:
            raise FuseOSError(errno.ENOENT)

    def readdir(self, path, fh):
        return ['.', '..'] + self.dirs.get(path, [])

    def read(self, path, size, offset, fh):
        if path not in self.files:
            raise FuseOSError(errno.ENOENT)
        content = self.files[path]
        if offset > len(content):
            return b''
        return content[offset:offset + size]

def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <json_file> <mount_point>")
        sys.exit(1)

    json_file = sys.argv[1]
    mount_point = sys.argv[2]

    if not os.path.isfile(json_file):
        print(f"Error: JSON file '{json_file}' not found")
        sys.exit(1)

    if not os.path.isdir(mount_point):
        print(f"Error: Mount point '{mount_point}' not found")
        sys.exit(1)

    with open(json_file, 'r') as f:
        try:
            json_data = json.load(f)
        except json.JSONDecodeError as e:
            print(f"Error parsing JSON: {e}")
            sys.exit(1)

    fuse = FUSE(JsonFs(json_data), mount_point, foreground=True, ro=True)

if __name__ == '__main__':
    main()