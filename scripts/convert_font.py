#!/usr/bin/env python3
"""
Convert an 8x8 font C header where each byte is a ROW into a header
where each byte is a COLUMN (transpose bits).

Usage:
  python scripts/convert_font.py input.h output.h
  python scripts/convert_font.py input.h --in-place

The script preserves parts of the file it doesn't understand and only
transforms lines that contain an 8-element hex byte initializer for
entries of the `font8x8_basic` array.
"""
import re
import argparse
from pathlib import Path

HEX_RE = re.compile(r'0x[0-9A-Fa-f]+')

def transpose_rows_to_cols(rows):
    # rows: list of 8 integers, each byte is a row where bit c is column c
    cols = []
    for c in range(8):
        col = 0
        for r in range(8):
            bit = (rows[r] >> c) & 1
            col |= (bit << r)
        cols.append(col)
    return cols

def format_glyph(cols, comment=None):
    hexes = [f'0x{b:02X}' for b in cols]
    line = '    { ' + ', '.join(hexes) + ' },'
    if comment:
        line = f'{line}   // {comment}'
    return line + '\n'

def convert_header(text):
    lines = text.splitlines(keepends=True)

    # Find start of the array
    start_idx = None
    for i, ln in enumerate(lines):
        if 'char' in ln and 'font8x8_basic' in ln and '=' in ln:
            # find next line with '{'
            for j in range(i, min(i+6, len(lines))):
                if '{' in lines[j]:
                    start_idx = j
                    break
            if start_idx is None:
                # maybe brace on same line
                if '{' in ln:
                    start_idx = i
            break

    if start_idx is None:
        raise RuntimeError('Could not find font8x8_basic array start')

    # find closing '};' after start
    end_idx = None
    for k in range(start_idx+1, len(lines)):
        if '};' in lines[k]:
            end_idx = k
            break
    if end_idx is None:
        raise RuntimeError('Could not find end of font8x8_basic array')

    out_lines = []
    out_lines.extend(lines[:start_idx+1])

    # process entries
    for ln in lines[start_idx+1:end_idx]:
        # preserve blank/comment lines
        if not ln.strip() or ln.strip().startswith('//'):
            out_lines.append(ln)
            continue

        # capture trailing comment (// ...)
        parts = ln.split('//', 1)
        data_part = parts[0]
        comment = parts[1].strip() if len(parts) > 1 else None

        hex_matches = HEX_RE.findall(data_part)
        if len(hex_matches) == 8:
            rows = [int(h, 16) for h in hex_matches]
            cols = transpose_rows_to_cols(rows)
            out_lines.append(format_glyph(cols, comment))
        else:
            # if the line doesn't match 8 hex numbers, preserve as-is
            out_lines.append(ln)

    out_lines.extend(lines[end_idx:])
    return ''.join(out_lines)

def main():
    p = argparse.ArgumentParser(description='Transpose 8x8 font rows->columns')
    p.add_argument('input', help='input C header file')
    p.add_argument('output', nargs='?', help='output file; if omitted and --in-place not set, prints to stdout')
    p.add_argument('--in-place', action='store_true', help='overwrite input file')
    args = p.parse_args()

    inp = Path(args.input)
    if not inp.exists():
        raise SystemExit(f'Input file not found: {inp}')

    original = inp.read_text(encoding='utf-8')
    converted = convert_header(original)

    if args.in_place:
        inp.write_text(converted, encoding='utf-8')
        print(f'Wrote in-place: {inp}')
    elif args.output:
        outp = Path(args.output)
        outp.write_text(converted, encoding='utf-8')
        print(f'Wrote: {outp}')
    else:
        print(converted)

if __name__ == '__main__':
    main()
