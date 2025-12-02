#ifndef LABEL_H
#define LABEL_H

// ============================================================================
// LABEL MANAGEMENT API
// ============================================================================

/**
 * Load labels from JSON file
 * @return 0 on success, -1 on failure
 */
int load_labels(void);

/**
 * Get a label by key path (e.g., "menu_items.admin")
 * @param key_path Dot-separated path to the label
 * @return Label text or NULL if not found
 */
const char* get_label(const char* key_path);

/**
 * Free all loaded labels
 */
void free_labels(void);

#endif
