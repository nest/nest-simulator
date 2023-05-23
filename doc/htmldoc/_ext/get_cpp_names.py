import json
import os
import re

def extract_compoundnames(filenames):
    compoundnames = []
    pattern = re.compile("<compoundname>(.*?)</compoundnaome>")
    for xml_file in filenames:
        if "classnest_1_1_" in xml_file and "nest_module_" not in  xml_file:
            for i, line in enumerate(open(xml_file)):
                for match  in re.finditer(pattern,  line):
                    print(match)
                #content = file.read()
                #matches = re.findall(pattern, content)
                #compoundnames.append(matches)
    return compoundnames

#def modify_filenames(filenames):
#    modified_filenames = []
#    for filename in filenames:
#        base_filename = os.path.basename(filename)  # Get only the filename without the directory path
#        if "classnest_1_1_" in base_filename and "nest_module_" not in  base_filename:
#            modified_filename = re.sub(r"classnest_1_1_|\.\w+$", "", base_filename)
#            #modified_filename = re.sub(r"classnest_1_1_nest_module_1_1_|__.*$", "", base_filename)
#            #modified_filename = os.path.splitext(base_filename)[0]  # Remove the extension
#            modified_filenames.append(modified_filename)
#    return modified_filenames
#
directory = "_doxygen/xml/"
filenames = [os.path.join(directory, filename) for filename in os.listdir(directory)]
#
#modified_filenames = modify_filenames(filenames)
#
## Save modified filenames as a list in a JSON file
#with open('cpp_output.json', 'w') as json_file:
#    json.dump(modified_filenames, json_file)
#
# Extract compound names from XML files
compoundnames = extract_compoundnames(filenames)

# Save compound names as a list in a JSON file
with open('cpp_output.json', 'w') as json_file:
    json.dump(compoundnames, json_file)

#In this script, the extract_compoundnames function takes a list of XML files and extracts the content between <compoundname> and </compoundname> using regular expressions. It reads each XML file, searches for matches using the provided pattern, and appends the matches to the compoundnames list. Finally, the compound names are stored as a JSON list in the "cpp-output.json" file.







