from collections import namedtuple;
import sys;
import fileinput;
import argparse;

# Parsing
MidiEntry = namedtuple('MidiEntry', 'track time command channel note velocity');

def parse(text):
    lines = str.split(text, "\n");
    result = [];
    for line in lines:
        exploded_line = list(map(lambda s: s.strip(), str.split(line, ',')));
        if len(exploded_line) == 6 and int(exploded_line[1]) != 0:
            result += [MidiEntry(track = int(exploded_line[0]), time = int(exploded_line[1]), command = exploded_line[2], channel = int(exploded_line[3]), note = int(exploded_line[4]), velocity = int(exploded_line[5]))];
    return result;

# Only for testing, actual text is passed to stdin
def parse_file(filename):
    f = open(filename);
    text = f.read();
    f.close();
    return parse(text);

# Translates an abstract midicsv file to an abstract sequence csv file
def get_sequence(commands, time_scale):
    commands = sorted(commands, key=lambda c: c.time);
    result = [['0', '00', '00', '00', '00', '00', '00', '00', '00']];
    prev_time = 0;
    for c in commands:
        time = c.time * time_scale - prev_time;
        prev_time = c.time * time_scale
        new_commands = [];
        if c.command == 'Note_on_c':
            if c.channel == 1:
                new_commands += [[str(time), '21',
                                  get_bit_string(c.note % 12 == 0,
                                                 c.note % 12 == 2,
                                                 c.note % 12 == 4,
                                                 c.note % 12 == 5,
                                                 c.note % 12 == 7,
                                                 c.note % 12 == 9,
                                                 c.note % 12 == 11,
                                                 False),
                                  get_bit_string(c.note % 12 == 1,
                                                 c.note % 12 == 3,
                                                 c.note % 12 == 6,
                                                 c.note % 12 == 8,
                                                 c.note % 12 == 10,
                                                 False, False, False), '00', '00', '00', '00', '00']];
            elif c.channel == 2:
                new_commands += [[str(time), '21', '00',
                                  get_bit_string(False, False, False, False, False,
                                                 c.note % 12 == 1,
                                                 c.note % 12 == 3,
                                                 c.note % 12 == 6),
                                  get_bit_string(c.note % 12 == 0,
                                                 c.note % 12 == 2,
                                                 c.note % 12 == 4,
                                                 c.note % 12 == 5,
                                                 c.note % 12 == 7,
                                                 c.note % 12 == 9,
                                                 c.note % 12 == 11,
                                                 c.note % 12 == 8), '00', '00', '00', '00']];
            elif c.channel == 3:
                new_commands += [[str(time), '31', '01',
                                  get_byte_string(c.velocity if c.note % 3 == 0 else 0),
                                  get_byte_string(c.velocity if c.note % 3 == 1 else 0),
                                  get_byte_string(c.velocity if c.note % 3 == 2 else 0), '00', '00', '00']];
            else:
                print("Warning: Channel", c.channel, "is unused");
        elif c.command == 'Note_off_c':
            if c.channel == 1:
                new_commands += [[str(time), '22',
                                  get_bit_string(c.note % 12 == 0,
                                                 c.note % 12 == 2,
                                                 c.note % 12 == 4,
                                                 c.note % 12 == 5,
                                                 c.note % 12 == 7,
                                                 c.note % 12 == 9,
                                                 c.note % 12 == 11,
                                                 False),
                                  get_bit_string(c.note % 12 == 1,
                                                 c.note % 12 == 3,
                                                 c.note % 12 == 6,
                                                 c.note % 12 == 8,
                                                 c.note % 12 == 10,
                                                 False, False, False), '00', '00', '00', '00', '00']];
            elif c.channel == 2:
                new_commands += [[str(time), '22', '00',
                                  get_bit_string(False, False, False, False, False,
                                                 c.note % 12 == 1,
                                                 c.note % 12 == 3,
                                                 c.note % 12 == 6),
                                  get_bit_string(c.note % 12 == 0,
                                                 c.note % 12 == 2,
                                                 c.note % 12 == 4,
                                                 c.note % 12 == 5,
                                                 c.note % 12 == 7,
                                                 c.note % 12 == 9,
                                                 c.note % 12 == 11,
                                                 c.note % 12 == 8), '00', '00', '00', '00']];
            elif c.channel == 3:
                pass;
            else:
                print("Warning: Channel", c.channel, "is unused");
        result += new_commands;
    result += [['5', '00', '00', '00', '00', '00', '00', '00', '00']];
    return result;

# get_sequence util
def get_bit_string(b1, b2, b3, b4, b5, b6, b7, b8):
    def trans_nibble(val):
        if val < 10:
            return str(val);
        elif val == 10:
            return 'A';
        elif val == 11:
            return 'B';
        elif val == 12:
            return 'C';
        elif val == 13:
            return 'D';
        elif val == 14:
            return 'E';
        elif val == 15:
            return 'F';
    upper = b1 * 8 + b2 * 4 + b3 * 2 + b4;
    lower = b5 * 8 + b6 * 4 + b7 * 2 + b8;
    return trans_nibble(upper) + trans_nibble(lower);

def get_byte_string(b):
    def trans_nibble(val):
        if val < 10:
            return str(val);
        elif val == 10:
            return 'A';
        elif val == 11:
            return 'B';
        elif val == 12:
            return 'C';
        elif val == 13:
            return 'D';
        elif val == 14:
            return 'E';
        elif val == 15:
            return 'F';
    return trans_nibble((b & 0xF0) // 16) + trans_nibble(b & 0x0F);

# Main driver
def make_csv(sequence):
    return '\n'.join(list(map(lambda l: ', '.join(l), sequence)));

parser = argparse.ArgumentParser();

#-ts TIME_SCALE
parser.add_argument("-ts", "--time_scale", dest = "time_scale", default = '1', help="Time scale to multiply each time step in the source to get seconds");
parser.add_argument("-t", "--tempo", dest = "tempo", default = '80', help="Tempo to use to automaticly calculate the time step");

args = parser.parse_args()

in_text = '';
for line in sys.stdin:
    in_text += line;

commands = parse(in_text);
sequence = get_sequence(commands, float(args.time_scale) if args.time_scale != '1' else 60 / (int(args.tempo) * 192));
out_text = make_csv(sequence);

print(out_text);
