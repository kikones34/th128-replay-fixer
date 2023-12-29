HEX_TO_BIN = {
    f"{i:X}": f"{i:04b}" for i in range(16)
}

def hex_string_to_binary_string(hex_string: str):
    return ''.join(HEX_TO_BIN[c] for c in hex_string.replace(" ", ""))


def parse_lzss(bits):
    chunks = []
    i = 0
    while i < len(bits):
        if bits[i] == "1":
            i += 1
            byte = int(bits[i:i+8], base=2)
            chunks.append(f"{byte:02X}")
            i += 8
        else:
            i += 1
            index = int(bits[i:i+13], base=2)
            if index == 0:
                chunks.append("EOF")
                break
            i += 13
            try:
                length = int(bits[i:i+4], base=2)
            except ValueError:
                chunks.append("Unexpected EOF")
                break
            chunks.append(f"({index - 1},{length + 3})")
            i += 4
    return ' '.join(chunks)


test = "AA 59 6E 77 49 04 82 41 20 00 38 65 DE AB A5 94 01 41 00 68 60 38 08 90 06 C3 00 81 80 85 E0 2A 78 0C DE 03 C7 81 15 E0 4E 78 15 DE 06 07 81 A5 E0 72 78 1E DE 08 47 82 35 E0 8F 78 23 DE 09 87 82 85 E0 AA 78 2C DE 0B C7 83 15 E0 CE 78 35 DE 0E 07 83 A5 E0 F2 78 3E DE 10 47 84 35 E1 16 78 47 DE 13 87 85 05 E1 4A 78 54 DA 00"
print(parse_lzss(hex_string_to_binary_string(test)))
