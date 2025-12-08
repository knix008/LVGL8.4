/**
 * @file json_utils.h
 * @brief Simple JSON parsing utilities
 *
 * Provides helper functions for extracting JSON sections from config files.
 * Designed for simple JSON parsing without external dependencies.
 */

#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Extract a JSON section by key name
 *
 * Finds a JSON object with the given key and extracts the entire key-value pair
 * including nested objects. Uses brace-matching to handle nested structures.
 *
 * Example:
 *   Input JSON: {"border": {"enabled": true, "color": "#FF0000"}, "theme": {...}}
 *   Key: "border"
 *   Output: "\"border\": {\"enabled\": true, \"color\": \"#FF0000\"}"
 *
 * @param json_str Source JSON string to search
 * @param key JSON key to find (e.g., "border", "theme")
 * @param output Buffer to store extracted section
 * @param output_size Size of output buffer
 * @return true if section was found and extracted, false otherwise
 */
bool json_extract_section(const char *json_str,
                          const char *key,
                          char *output,
                          size_t output_size);

/**
 * @brief Find matching closing brace for a JSON object
 *
 * Given a pointer to an opening brace '{', finds the matching closing brace '}'
 * by tracking nesting depth.
 *
 * @param opening_brace Pointer to opening brace character
 * @return Pointer to matching closing brace, or NULL if not found
 */
const char *json_find_closing_brace(const char *opening_brace);

#endif // JSON_UTILS_H
