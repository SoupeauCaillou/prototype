#!/usr/bin/env python
# coding: utf-8

import os
import sys
import time
import argparse
from math import cos
from math import sin
from math import radians

def write_file(filename, data=""):
    print "Writing data in file '{0}'.".format(filename)
    f = open(filename, "w")
    f.write(data)
    f.close()

def write_hardcoded_section( width, height ):
    sections = \
"""[sheep]
objective_arrived   = 5
objective_survived  = 5
objective_time_limit    = 600

# cette section pourrait etre hardcodee aussi
[wall]
count = 4
# haut
position_1%gimp = {2}, 0
size_1%gimp = {0}, 0
# bas
position_2%gimp = {2}, {1}
size_2%gimp = {0}, 0
# gauche
position_3%gimp = 0, {3}
size_3%gimp = 0, {1}
#droite
position_4%gimp = {0}, {3}
size_4%gimp = 0, {0}\n""".format( width, height, width/2, height/2)
    return sections

def write_polygon_section(section, section_name):
    if len(section) == 0:
        return []
    sections = ["[{0}]".format(section_name)]
    sections.append("count = {}".format(len(section)))
    count = 0
    for element in section:
        count = count + 1
        sections.append("polygon_{0}%gimp = {1}".format(count, ', '.join(str(e) for e in element)))
    
    sections.append("")
    return sections


def parse_file(xcf_filename, directory):
    from gimpfu import pdb
    from gimpfu import gimp

    # load xcf file in gimp
    image = pdb.gimp_file_load( os.path.join(directory, xcf_filename), xcf_filename)

    sections = []

    # looking for paths
    obstacle_section = []
    zone_section = []
    others_section = []

    for vector in image.vectors:
        for stroke in vector.strokes:              
            points = map(int, stroke.points[0])
            if "obstacle" in vector.name:
                obstacle_section.append( points )
            elif "zone" in vector.name:
                zone_section.append( points )
            else:
                others_section.append( points )

    sections += [write_hardcoded_section(image.width, image.height)]
    sections += write_polygon_section(obstacle_section, "obstacle")
    sections += write_polygon_section(zone_section, "zone")
    sections += write_polygon_section(others_section, "others")
    sections.append("")
    
    return sections


def run(filename, directory, output):

    start = time.time()

    write_file(os.path.join(directory, output, "level.ini"), "\n".join( parse_file(filename, directory) ) )
    
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

    if os.path.exists( os.path.join(args.output, "level.ini") ):
        print("File '{0}' already exists. Please remove it first.".format( os.path.join(args.output, "level.ini") ) )
        sys.exit(3)

    # create output directory
    directory = os.getcwd()
    if not os.path.isdir(args.output):
        os.mkdir(args.output)

    script_directory = os.path.abspath( os.path.dirname(sys.argv[0]) )

    # execute gimp with this script
    os.chdir(script_directory)

    command = 'gimp -idfs --batch-interpreter python-fu-eval ' \
              '-b "import sys; sys.path=[\'.\']+sys.path; ' \
              'import generate_level as e; e.run(\'' + args.filename + '\',\'' + directory + '\',\'' + args.output + '\')" ' \
              '-b "pdb.gimp_quit(1)"'

    os.system(command)
    os.chdir(directory)
