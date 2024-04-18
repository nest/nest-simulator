import glob
import json
import os
from collections import Counter
from itertools import combinations


def find_models_in_tag_combinations(models_dict):
    # Reverse the models_dict to map tags to models
    model_to_tags = {}
    for model, tags in models_dict.items():
        for tag in tags:
            if tag not in model_to_tags:
                model_to_tags[tag] = set()
            model_to_tags[tag].add(model)

        # Prepare the list that will eventually be converted to JSON
    result_list = []

    for tag, models in model_to_tags.items():
        # Create a dictionary for each tag-model pair
        if isinstance(models, set):
            models = list(models)  # Convert set to list

        tag_info = {"tag": tag, "models": models, "count": len(models)}  # Number of models
        result_list.append(tag_info)

        # Write the JSON output directly to a file
    with open("_static/filter_model.json", "w") as json_file:
        json.dump(result_list, json_file, indent=2)
    # Prepare for combinations of tags
    tags = list(model_to_tags.keys())

    # Dictionary to hold the result of combinations
    combination_results = {}

    # Generate combinations of tags from 2 to the length of 4
    for r in range(2, 5):
        for combo in combinations(tags, r):
            combo_key = "_AND_".join(combo)
            # combination_results[combo_key] = []

            models_for_combo = []
            # Check each model to see if it fits in the current combination
            for model, model_tags in models_dict.items():
                if all(tag in model_tags for tag in combo):
                    models_for_combo.append(model)
            # Only add to combination_results if models_for_combo is not empty
            if models_for_combo:
                combination_results[combo_key] = models_for_combo
    # Step 1: Count the number of models for each tag combination

    combination_counts = {key: len(models) for key, models in combination_results.items()}

    # Step 2: Sort the keys based on the number of models in descending order
    sorted_combination_keys = sorted(combination_counts, key=combination_counts.get, reverse=True)

    # Step 3: Create a new dictionary with sorted keys
    sorted_combination_results = {key: combination_results[key] for key in sorted_combination_keys}

    return sorted_combination_results

    # Optional: Print the sorted results with counts for verification
    # for key, models in sorted_combination_results.items():
    #    print(f"{key} ({len(models)} models): {models}")

    # Now find and store keys with identical model lists


def extract_tags_from_files(directory_path):
    """
    Iterates through a list of file paths, opens each file, and extracts tags from the line that contains 'BeginUserDocs'.
    Tags are expected to be comma-separated right after 'BeginUserDocs'.

    Parameters:
    - file_paths: A list of strings, where each string is a path to a file.

    Returns:
    A dictionary with filenames as keys and lists of tags as values.
    """
    models_dict = {}

    file_paths = glob.glob(os.path.join(directory_path, "*.h"))

    for file_path in file_paths:
        try:
            with open(file_path, "r") as file:
                # Split the path into a directory and file name
                directory, filename = os.path.split(file_path)

                # Check if the file extension is '.h' and replace it with '.html'
                if filename.endswith(".h"):
                    model_name = filename[:-2]  # Remove the last two characters '.h'
                    new_filename = f"{model_name}.html"  # Append .html instead of .h
                    new_path = os.path.join("models/", new_filename)  # Reconstruct the full path with new filename
                    formatted_path = f"{new_path}"  # Assuming all paths should start with /models/

                # Initialize with no tags for the file
                models_dict[formatted_path] = []

                for line in file:
                    if "BeginUserDocs" in line:
                        # Assuming tags are after 'BeginUserDocs' and are comma-separated
                        parts = line.split("BeginUserDocs:", 1)
                        if len(parts) > 1:
                            tags = parts[1].strip().split(",")
                            # Strip whitespace from each tag and filter out empty strings
                            # Strip whitespace from each tag, replace spaces with underscores, and filter out empty strings
                            tags = [tag.strip().replace(" ", "_") for tag in tags if tag.strip()]
                            models_dict[formatted_path] = tags
                            break  # Assuming only one line contains 'BeginUserDocs'
        except FileNotFoundError:
            print(f"File {file_path} not found.")
        except Exception as e:
            print(f"An error occurred while processing {file_path}: {e}")

    return models_dict


# Example usage


def get_models(app, env, docname):
    directory_path = "../../models/"

    if not hasattr(env, "combo_dict"):
        env.combo_dict = {}

    models_dict = extract_tags_from_files(directory_path)
    find_models_in_tag_combinations(models_dict)
    env.combo_dict = models_dict
    # Calculate combinations and print results


# For demonstration, here's how to print the results
# for combo, models in combination_results.items():
#    print(f"{combo}: {models}")
def model_customizer(app, docname, source):
    env = app.builder.env
    if docname == "test_model":
        get_tags = env.combo_dict
        html_context = {"tag_dict": get_tags}
        model_source = source[0]
        rendered = app.builder.templates.render_string(model_source, html_context)
        source[0] = rendered


def setup(app):
    app.connect("env-before-read-docs", get_models)
    app.connect("source-read", model_customizer)

    return {
        "version": "0.1",
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
