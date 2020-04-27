import argparse
import importlib.util
import os
import shutil

spec = importlib.util.spec_from_file_location('setup_utils',
  os.path.join('..', '..', 'Python', 'setup_utils.py'))
setup_utils = importlib.util.module_from_spec(spec)
spec.loader.exec_module(setup_utils)


def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2020 Spire Trading Inc.')
  parser.add_argument('-l', '--local', type=str, help='Local interface.',
    default=setup_utils.get_ip())
  args = parser.parse_args()
  variables = {}
  variables['local_interface'] = args.local
  shutil.copy('config.default.yml', 'config.yml')
  with open('config.yml', 'r+') as file:
    source = setup_utils.translate(file.read(), variables)
    file.seek(0)
    file.write(source)
    file.truncate()


if __name__ == '__main__':
  main()
