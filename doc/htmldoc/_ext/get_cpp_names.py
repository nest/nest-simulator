import json
import os
import re


def extract_compoundnames(filenames):
    compoundnames = []
    pattern = re.compile("<compoundname>(nest::.*)</")

    for filename in filenames:
        with open(filename) as f:
            file_contents = f.read()
            for match in pattern.findall(file_contents):
                compoundnames.append(match)

    return compoundnames


def process_files(directory):
    result = {}
    for filename in os.listdir(directory):
        if filename.endswith(".h"):
            file_path = os.path.join(directory, filename)
            with open(file_path, "r") as file:
                content = file.read()
                start_index = content.find("specialtag_")
                end_index = content.find("_end", start_index)
                if start_index != -1 and end_index != -1:
                    match_string = content[
                        start_index + len("specialtag_") : end_index
                    ].strip()
                    result[filename] = match_string.split(" ")
    return result


# Example usage
directory_path = "../../nestkernel"
output_file_path = "test_list.json"

result_dict = process_files(directory_path)

with open(output_file_path, "w") as output_file:
    json.dump(result_dict, output_file)

# def extract_specialtag(filenames):
#    specialtags = []
#    pattern = re.compile("specialtag_(.*)_end")
#
#    for filename in filenames:
#        print(filename)
#        with open(filename) as f:
#            file_contents = f.read()
#            for match in pattern.findall(file_contents):
#                specialtags.append(match)
#
#    return specialtags
#
#    for xml_file in filenames:
#        for line in open(xml_file):
#            result = re.findall(r"<compoundname>(nest::.*?)</compoundname>",line)
#                #result = pattern.search(line)
#            print(result)
# print(result.group(1))


# compound_name = re.search(pattern, xml_file
# for i, line in enumerate(open(xml_file)):
#    for match  in re.finditer(pattern,  line):
# content = file.read()
# matches = re.findall(pattern, content)
#        compoundnames.append(match)
# return compoundnames

directory = "_doxygen/xml/"
filenames = [os.path.join(directory, filename) for filename in os.listdir(directory)]
#
#
#
# Extract compound names from XML files
compoundnames = extract_compoundnames(filenames)
# specialtags = extract_specialtag(filenames)

# Save compound names as a list in a JSON file
with open("cpp_output.json", "w") as json_file:
    json.dump(compoundnames, json_file)

# with open('specialtags.json', 'w') as json_file:
#    json.dump(specialtags, json_file)

# In this script, the extract_compoundnames function takes a list of XML files and extracts the content between <compoundname> and </compoundname> using regular expressions. It reads each XML file, searches for matches using the provided pattern, and appends the matches to the compoundnames list. Finally, the compound names are stored as a JSON list in the "cpp-output.json" file.
