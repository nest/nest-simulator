document.addEventListener('DOMContentLoaded', function() {
    fetch('filter_models.json')
        .then(response => response.json())
        .then(data => {
            populateTags(data);
        })
        .catch(error => console.error('Error loading the JSON data:', error));
});

function populateTags(data) {
    const select = document.getElementById('tag-select');
    data.forEach(item => {
        const option = document.createElement('option');
        option.value = item.tag;
        option.textContent = item.tag;
        select.appendChild(option);
    });
}

function updateModelDisplay() {
    const select = document.getElementById('tag-select');
    const selectedOptions = Array.from(select.selectedOptions).map(opt => opt.value);
    fetch('filter_models.json')
        .then(response => response.json())
        .then(data => {
            const filteredModels = filterModelsByTags(data, selectedOptions);
            displayModels(filteredModels);
        });
}

function filterModelsByTags(data, selectedTags) {
    const tagModelMap = new Map(data.map(item => [item.tag, item.models]));
    let intersection = [];
    selectedTags.forEach((tag, index) => {
        if (index === 0) {
            intersection = tagModelMap.get(tag);
        } else {
            intersection = intersection.filter(model =>
              tagModelMap.get(tag).includes(model));
        }
    });
    return intersection;
}

function displayModels(models) {
    const modelList = document.getElementById('model-list');
    modelList.innerHTML = models.join('<br>');
}
