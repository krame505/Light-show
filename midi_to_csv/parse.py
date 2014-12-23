from collections import namedtuple;

MidiEntry = namedtuple('MidiEntry', 'track time command channel note velocity');

def parse(text):
    lines = str.split(text, "\n");
    result = set();
    for line in lines:
        exploded_line = str.split(line, ', ');
        if len(exploded_line) == 6 and int(exploded_line[1]) != 0:
            result = result.union(set(MidiEntry(track = exploded_line[0], time = int(exploded_line[1]), command = exploded_line[2], channel = int(exploded_line[3]), note = int(exploded_line[4]), velocity = int(exploded_line[5]))));
    return result;

def parse_file(filename):
    f = open(filename);
    text = f.read();
    f.close();
    return parse(text);

print(parse_file('../../music/carolbel.csv'));
