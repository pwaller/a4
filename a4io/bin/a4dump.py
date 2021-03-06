#!/usr/bin/env python
from sys import argv, exit
from a4.stream import InputStream
from optparse import OptionParser

parser = OptionParser()
parser.add_option("-s", "--scan", action="store_true", help="wait for keypress to show next event")
parser.add_option("-e", "--event", action="append", default=[], help="event number to dump - can be specified multiple times")
parser.add_option("-i", "--index", default=1, help="index of the (first) event to dump")
parser.add_option("-a", "--all", action="store_true", help="dump all events")
(options, args) = parser.parse_args()

if len(args) == 0:
    parser.print_help()
    exit(-1)

skip = int(options.index)-1
events = map(int, options.event)
do_events = (len(events) > 0)

for fn in args:
    infile = InputStream(open(fn))
    print("%s: %s" % (fn, infile.info()))

    for obj in infile:
        if skip > 0:
            skip -= 1
            continue
        if options.all:
            print obj
            if options.scan:
                raw_input("Press Enter to continue, CTRL+C to abort...")
        elif do_events:
            if hasattr(obj, "event_number") and obj.event_number in events:
                print obj
                events.remove(obj.event_number)
                if len(events) == 0:
                    exit(0)
                if options.scan:
                    raw_input("Press Enter to continue, CTRL+C to abort...")
        elif not options.scan:
            print obj
            exit(0)
        else:
            print obj
            if options.scan:
                raw_input("Press Enter to continue, CTRL+C to abort...")

