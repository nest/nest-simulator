// Wait for the DOM to be fully loaded before executing the script
document.addEventListener('DOMContentLoaded', function() {
    // Fetch the JSON data containing model tags and counts
    fetch('../_static/data/filter_model.json')
        .then(response => response.json())
        .then(data => {
            // Populate the tags on the page with the fetched data
            populateTags(data);
        })
        .catch(error => console.error('Error loading the JSON data:', error));
});

/**
 * Populates the tag buttons on the page.
 *
 * This function creates buttons for each tag in the provided data,
 * giving priority to certain tags. Tags are sorted and displayed
 * accordingly.
 *
 * @param {Array} data - Array of objects containing tags and their counts.
 */
function populateTags(data) {
    const container = document.getElementById('tag-container');
    container.innerHTML = ''; // Ensure no duplicates

    const priorityTags = ['neuron', 'synapse', 'device'];
    const priorityData = [];
    const otherData = [];

    // Split data into priority and other based on tags
    data.forEach(item => {
        if (priorityTags.includes(item.tag)) {
            priorityData.push(item);
        } else {
            otherData.push(item);
        }
    });

    // Sort otherData by count in descending order
    otherData.sort((a, b) => b.count - a.count);

    // Merge arrays for rendering
    const finalData = [...priorityData, ...otherData];

    // Render buttons for each tag
    finalData.forEach(item => {
        const button = document.createElement('button');
        button.className = 'filter-button';
        button.textContent = `${item.tag} (${item.count})`;
        button.dataset.tag = item.tag; // Store the tag as a data attribute
        button.onclick = function() {
            this.classList.toggle('is-active');
            updateModelDisplay();
        };
        container.appendChild(button);
    });
}

/**
 * Updates the model display based on the selected tags.
 *
 * This function fetches the JSON data again, filters the models based on
 * the active tags, and then displays the filtered models.
 */
function updateModelDisplay() {
    const activeButtons = document.querySelectorAll('.filter-button.is-active');
    const selectedTags = Array.from(activeButtons).map(button => button.dataset.tag); // Use data attribute for tags
    fetch('../_static/data/filter_model.json')
        .then(response => response.json())
        .then(data => {
            const filteredModels = filterModelsByTags(data, selectedTags);
            displayModels(filteredModels);
        });
}

/**
 * Filters the models based on the selected tags.
 *
 * This function creates an intersection of models that match all selected tags.
 *
 * @param {Array} data - Array of objects containing tags and their models.
 * @param {Array} selectedTags - Array of tags selected by the user.
 * @returns {Array} - Array of models that match all selected tags.
 */
function filterModelsByTags(data, selectedTags) {
    const tagModelMap = new Map(data.map(item => [item.tag, item.models]));
    let intersection = selectedTags.reduce((acc, tag, index) => {
        if (index === 0) {
            return tagModelMap.get(tag) || [];
        } else {
            return acc.filter(model => tagModelMap.get(tag).includes(model));
        }
    }, []);
    return intersection;
}

/**
 * Displays the filtered models on the page.
 *
 * This function updates the DOM to show the models that match the selected tags,
 * fetching the HTML content for each model and displaying it as a list.
 *
 * @param {Array} models - Array of model URLs to be displayed.
 */
function displayModels(models) {
    const modelList = document.getElementById('model-list');
    modelList.innerHTML = '';  // Clear previous content

    if (models.length === 0) {
        modelList.innerHTML = "Sorry, we couldn't find any results. Try another combination of tags";
    } else {
        models.sort();  // Sort models alphabetically by their URLs

        // Create an unordered list element
        const ul = document.createElement('ul');
        modelList.appendChild(ul);  // Append the list to the model list container

        // Process each model
        models.forEach(model => {
            // Fetch the HTML content of the model
            fetch(model)
                .then(response => response.text())
                .then(htmlContent => {
                    // Use DOMParser to parse the HTML content
                    const parser = new DOMParser();
                    const doc = parser.parseFromString(htmlContent, "text/html");
                    let firstHeading = doc.querySelector("h1") ? doc.querySelector("h1").textContent : "No Heading";

                    // Remove pilcrow character
                    firstHeading = firstHeading.replace(/\W$/g, '');
                    // Create the link with model name and first heading
                    const link = document.createElement('a');
                    const modelBaseName = model.split('/').pop().replace('.html', ''); // Extract base name for display
                    link.href = model;  // Use the full model path directly
                    link.textContent = `${firstHeading}`; // Include the first heading
                    link.className = 'model-link'; // For CSS styling
                    // link.target = '_blank'; // Opens link in a new tab

                    // Create a list item and append the link
                    const li = document.createElement('li');
                    li.appendChild(link);
                    ul.appendChild(li);
                })
                .catch(error => {
                    console.error('Failed to load model content:', error);
                    const li = document.createElement('li');
                    li.textContent = `Failed to load: ${model}`;
                    ul.appendChild(li);
                });
        });
    }
}
