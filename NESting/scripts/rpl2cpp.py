import argparse
import re
import sys
import os.path
from base64 import b64decode


PRESET_PATTERN = re.compile(r'<PRESET `(.+)`\s+([^<>]+)>')
SPACE_PATTERN = re.compile(r'\s+')
NON_NAME_PATTERN = re.compile(r'[^A-Za-z0-9_]')
LINE_WRAP_CHARS = ' ,'
LINE_WRAP_AMOUNT = 120  # TODO make this a parameter
INDENTATION = 2
PRESET_STRUCT_DECL = '''struct rp_preset_t {
  rp_preset_t(const char* name, const uint8_t* data, uint32_t size) : name(name), data(data), size(size) {}
  const char* name;
  const uint8_t* data;
  const uint32_t size;
};'''

class Preset:
  '''
  A simple class represeting a preset from an input preset file.
  '''
  def __init__(self, name, c_name, size):
    self.name = name
    self.c_name = c_name
    self.size = size

def line_wrap(msg, max_len, indent):
  indent_str = ' ' * indent
  out = ''
  while len(msg) > max_len:
    # Go to the end of a line and walk backwards until we find a char we can split on
    i = max_len
    while i >= 0 and LINE_WRAP_CHARS.find(msg[i]) == -1:
      i -= 1
    # If the line cannot be split (e.g. VERY long preset name), go forwards until we can split
    if i == -1:
      i = max_len
      while LINE_WRAP_CHARS.find(msg[i]) == -1:
        i += 1
    # We found a char to split at, but we want to split AFTER it
    i += 1
    out += msg[0:i] + '\n' + indent_str
    # And move over to the next part of the message
    msg = msg[i:]
  out += msg
  return out


def process(input_file, output_name, output_dir, prefix, list_name):
  input_text = open(input_file, 'r').read()
  # List of Preset objects
  preset_info = []

  hdr_name = output_name + '.h'
  src_name = output_name + '.cpp'
  list_struct_name = 'rp_preset_t'

  out_fd = open(os.path.join(output_dir, src_name), 'w')

  # Include our header file, we'll need it
  out_fd.write('#include "%s"\n\n' % (hdr_name))
  # Now write each piece of info 
  for m in PRESET_PATTERN.finditer(input_text):
    p_name = m.group(1)
    c_name = re.sub(NON_NAME_PATTERN, '_', p_name)
    p_data = re.sub(SPACE_PATTERN, '', m.group(2))
    # p_data is base64-encoded, fix that
    b_data = b64decode(p_data)

    # Now write out the data as const char*
    # First we create the declaration and the name
    const_name = prefix + c_name
    # Record the name for later, in case we need to make a header
    preset_info.append(Preset(p_name, const_name, len(b_data)))
    # Write the header for this preset
    msg = 'const uint8_t %s[%d] = {' % (const_name, len(b_data))
    # Conver the bytes into a C array
    for b in b_data:
      msg += str(b) + ','
    msg += '};\n\n'
    # Now line wrap and write to the file
    out_fd.write(line_wrap(msg, LINE_WRAP_AMOUNT, INDENTATION))
  # Write an empty line
  out_fd.write('\n')
  # Write the preset list header
  out_fd.write('const %s %s[%d] = {\n' % (list_struct_name, list_name, len(preset_info) + 1))
  # Write each value in the preset list
  indent_str = ' ' * INDENTATION
  for preset in preset_info:
    msg = '%s("%s", %s, %d),\n' % (list_struct_name, preset.name, preset.c_name, preset.size)
    out_fd.write(indent_str + msg)
  # For our final "preset" we write one will all 0 values. This indicates the end of the list.
  out_fd.write(indent_str + '%s(NULL, NULL, 0),\n};' % (list_struct_name))
  out_fd.close()

  # Write the header file
  with open(os.path.join(output_dir, hdr_name), 'w') as fd:
    fd.write('#pragma once\n#include <stdint.h>\n\n')
    fd.write(PRESET_STRUCT_DECL)
    fd.write('\n')
    for preset in preset_info:
      fd.write('extern const uint8_t {}[{}];\n'.format(preset.c_name, preset.size))
    fd.write('extern const {} {}[{}];\n'.format(list_struct_name, list_name, len(preset_info) + 1))
    fd.write('\n')

def main(argv):
  parser = argparse.ArgumentParser("rpl2cpp.py")
  parser.add_argument('input', type=str,
    help='Input .RPL file')
  parser.add_argument('-n', '--name', type=str, default='presets',
    help='Name of output files (e.g. test would create test.cpp and test.h) (default: presets)')
  parser.add_argument('-o', '--output', type=str, default='.',
    help='Output directory for source and header files')
  parser.add_argument('-p', '--prefix', type=str, default='PRESET_',
    help='Prefix for the constant names (default: PRESET_)')
  parser.add_argument('--list', type=str, default='PRESET_LIST',
    help='Name of an array of structs representing each preset in order (default: PRESET_LIST)')
  args = parser.parse_args(argv)

  process(args.input, args.name, args.output, args.prefix, args.list)


if __name__ == '__main__':
  main(sys.argv[1:])

