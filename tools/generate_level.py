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


def write_section(section, section_name):
    sections = ["[{0}]".format(section_name)]
    count = 0
    for element in section:
        count = count + 1
        for key in element.keys():
            sections.append("{0}_{1}_{2}\t=\t{3}".format(section_name, key ,count, ', '.join(str(e) for e in element[key])))
    
    sections.append("")
    return sections


def parse_file(xcf_filename, directory):
    from gimpfu import pdb
    from gimpfu import gimp

    # load xcf file in gimp
    image = pdb.gimp_file_load( os.path.join(directory, xcf_filename), xcf_filename)

    sections = []
    # sheep_section = []
    # bush_section = []
    # wall_section = []
    # zone_section = []
    # objectives_section = []

    # current_group = ""
    # #Looking in all layers (included children)
    # for layer in image.layers:
    #     current_group = ""
    #     for l in [layer] + layer.children:
    #         if type(l) is gimp.GroupLayer or l.children != []:
    #             print "one more group", l.name
    #             current_group = l.name
    #             continue

    #         if "objectifs" in current_group.lower():
    #             objectives_section.append(l.name.lower())
    #             continue

    #         rotation = 0.0
    #         sizeX = l.width
    #         sizeY = l.height
    #         centerX = l.offsets[0] + sizeX / 2
    #         centerY = l.offsets[1] + sizeY / 2
    #         if "_" in l.name.lower():
    #             rotation = float(l.name.split("_")[-1])

    #         sizeX = sizeX * cos(radians(rotation))
    #         sizeY = sizeY * sin(radians(rotation))
    #         t = { "position": (centerX, centerY), "size": (sizeX, sizeY), "rotation": rotation }

    #         if "moutons" in current_group.lower():
    #             sheep_section.append(t)
    #         elif "buisson" in current_group.lower():
    #             bush_section.append(t)
    #         elif "barriere" in current_group.lower():
    #             wall_section.append(t)
    #         elif "zone" in current_group.lower():
    #             zone_section.append(t)
    #         else:
    #             print("Warning: Undefined group for layer {0}. This layer\
    #                  will be ignored.".format(l.name) )

    # looking for paths
    polygon_section = []

    for vector in image.vectors:
        print vector.strokes
        for stroke in vector.strokes:              
            points = {vector.name: map(int, stroke.points[0])}
            if points not in polygon_section:
                polygon_section.append( points )
    
    # print polygon_section


    # for o in objectives_section:
    #     sections.append(o)
    # sections += write_section(sheep_section, "sheep")
    # sections += write_section(bush_section, "bush")
    # sections += write_section(wall_section, "wall")
    # sections += write_section(zone_section, "zone")
    sections += write_section(polygon_section, "polygon")
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
