# -*- coding: utf-8 -*-

# This script builds the .h/.cpp files for managing the placement of
# applications on the home menu, from the apps_layout.csv file.

import argparse
import csv
import io

parser = argparse.ArgumentParser(description="Build tools for the placement of applications on the home menu.")
parser.add_argument('--header', help='the .h file to generate')
parser.add_argument('--implementation', help='the .cpp file to generate')
parser.add_argument('--apps', nargs='+', help='apps that are actually compiled')
parser.add_argument('--layouts', help='the apps_layout.csv file')

args = parser.parse_args()

def build_permutation(apps, appsOrdered):
    res = [0] * len(apps)
    i = 0
    for app in appsOrdered:
        if app in apps:
            res[i] = apps.index(app) + 1
            i += 1
    return res

def parse_config_file(layouts, apps):
    res = {'styles':[], 'permutations':[]}
    with io.open(layouts, "r", encoding="utf-8") as f:
        csvreader = csv.reader(f, delimiter=',')
        for row in csvreader:
            res['styles'].append(row[0])
            res['permutations'].append(build_permutation(apps, row[1:]))
    return res

data = parse_config_file(args.layouts, args.apps)

def print_header(header, data):
    f = open(header, "w")
    f.write("#ifndef HOME_APPS_LAYOUT_H\n")
    f.write("#define HOME_APPS_LAYOUT_H\n")
    f.write("// This file is auto-generated by apps_layout.py\n\n")
    f.write("namespace Home {\n\n")

    f.write("int PermutedAppSnapshotIndex(int index);\n\n")

    f.write("}\n\n")
    f.write("#endif")
    f.close()

def print_implementation(implementation, data):
    f = open(implementation, "w")
    f.write("// This file is auto-generated by apps_layout.py\n\n")
    f.write('#include "apps_layout.h"\n')
    f.write("#include <apps/country_preferences.h>\n")
    f.write("#include <apps/global_preferences.h>\n")
    f.write("#include <assert.h>\n\n")
    f.write("namespace Home {\n\n")

    styles = data['styles']
    permutations = data['permutations']

    f.write("/* Permutations are built so that Permutation[n] is the index of the snapshot\n")
    f.write(" * for the nth app in the Home menu. */\n\n")

    for i in range(len(styles)):
        f.write("constexpr static int " + styles[i] + "AppsPermutation[] = {\n")
        f.write("  0,\n")
        for j in permutations[i]:
            f.write("  " + str(j) + ",\n")
        f.write("};\n")
        f.write('static_assert(' + styles[i] + 'AppsPermutation[0] == 0, "The Home apps must always be at index 0");\n\n')

    f.write("int PermutedAppSnapshotIndex(int index) {\n")
    f.write("  CountryPreferences::HomeAppsLayout currentLayout = GlobalPreferences::sharedGlobalPreferences()->homeAppsLayout();\n")
    for i in range(len(styles)):
        f.write("  if (currentLayout == CountryPreferences::HomeAppsLayout::" + styles[i] + ") {\n")
        f.write("    return " + styles[i] + "AppsPermutation[index];")
        f.write("  }\n")
    f.write("  assert(false);\n")
    f.write("  return -1;\n")
    f.write("}\n\n")

    f.write("}\n\n")
    f.close()

if args.header:
    print_header(args.header, data)
if args.implementation:
    print_implementation(args.implementation, data)
