#!/usr/bin/env python
# coding: utf-8

import os
import sys
import time
import argparse


def write_file(filename, data=""):
    print "Writing data in file '{0}'.".format(filename)
    f = open(filename, "w")
    f.write(data)
    f.close()


def write_section(sections, section, section_name):
    sections.append("[{0}]".format(section_name))
    count = 0
    for e in section:
        count = count + 1
        sections.append("{0}_position_{1}\t=\t{2}, {3}".format(section_name, count, e[0], e[1]))
        sections.append("{0}_size_{1}\t=\t{2}, {3}".format(section_name, count, e[2], e[3]))
        sections.append("{0}_rotation_{1}\t=\t{2}".format(section_name, count, e[4]))
        sections.append("")

def parse_file(xcf_filename):
    from gimpfu import pdb
    from gimpfu import gimp

    # load xcf file in gimp
    image = pdb.gimp_file_load(xcf_filename, xcf_filename)

    sections = []
    sheep_section = []
    bush_section = []
    wall_section = []
    zone_section = []
    objectives_section = []

    current_group = ""
    #Looking in all layers (included children)
    for layer in image.layers:
        current_group = ""
        for l in [layer] + layer.children:
            if type(l) is gimp.GroupLayer:
                current_group = l.name
                continue

            if "objective" in current_group.lower():
                objectives_section.append(l.name.lower())

            rotation = 0
            centerX = l.offsets[0] + l.width / 2
            centerY = l.offsets[1] + l.height / 2

            if "_" in l.name.lower():
                rotation = l.name.split("_")[-1]

            t = (centerX, centerY, l.width, l.height, rotation)

            if "sheep" in current_group.lower():
                sheep_section.append(t)
            elif "bush" in current_group.lower():
                bush_section.append(t)
            elif "wall" in current_group.lower():
                wall_section.append(t)
            elif "zone" in current_group.lower():
                zone_section.append(t)
            else:
                print "Warning: Undefined group for layer {0}. This layer" \
                	" will be ignored.".format(l.name)


    for o in objectives_section:
        sections.append(o)
    write_section(sections, sheep_section, "sheep")
    write_section(sections, bush_section, "bush")
    write_section(sections, wall_section, "wall")
    write_section(sections, zone_section, "zone")

    return sections


def run(filename, directory, output):

    start = time.time()

    write_file(output + "/level.ini", "\n".join(parse_file(filename)))
    end = time.time()
    print("Finished, total processing time: {:.2f}".format(end - start))

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=
        """This script will generate a INI level file from a given
        gimp XCF file by extracting layers accoding to a convention.""")
    parser.add_argument('-i', metavar="input-file", type=str, required=True,
        help="""Path of input file to convert""", dest="filename")
    parser.add_argument('-o', metavar="output-directory", type=str,
        help="""Path of output file""", dest="output", required=True)

    args = parser.parse_args()

    if not os.path.exists(args.filename):
        print("File '{0}' does not exist.".format(args.filename))
        sys.exit(1)
    elif ".xcf" not in args.filename:
        print("File '{0}' does not have valid extension (.xcf).".format(args.filename))
        sys.exit(2)

    if os.path.exists(args.output + "/level.ini"):
        print("File '{0}' already exists. Please remove it first.".format(args.output + "/level.ini"))
        sys.exit(3)

    # create output directory
    directory = os.getcwd()
    if not os.path.isdir(args.output):
        os.mkdir(args.output)

    script_directory = os.path.abspath(os.path.dirname(sys.argv[0]))

    # execute gimp with this script
    os.chdir(script_directory)

    command = 'gimp -idfs --batch-interpreter python-fu-eval ' \
              '-b "import sys; sys.path=[\'.\']+sys.path; ' \
              'import generate_level as e; e.run(\'' + args.filename + '\',\'' + directory + '\',\'' + args.output + '\')" ' \
              '-b "pdb.gimp_quit(1)"'

    os.system(command)
    os.chdir(directory)
