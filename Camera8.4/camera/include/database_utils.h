#ifndef DATABASE_UTILS_H
#define DATABASE_UTILS_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <map>
#include <functional>

/**
 * @file database_utils.h
 * @brief Database utility functions to reduce duplication
 *
 * Provides helper functions for common database operations like
 * statement preparation, result binding, and data extraction.
 */

/**
 * @brief Database utilities namespace
 *
 * Contains helper functions for SQLite operations to reduce code duplication
 * across the codebase.
 */
namespace DatabaseUtils {

    /**
     * @brief SQLite statement wrapper for RAII pattern
     *
     * Automatically finalizes prepared statement when going out of scope.
     */
    class Statement {
    private:
        sqlite3_stmt* stmt;
        bool is_prepared;

    public:
        /**
         * @brief Construct statement wrapper
         *
         * @param prepared_stmt Prepared SQLite statement
         */
        explicit Statement(sqlite3_stmt* prepared_stmt)
            : stmt(prepared_stmt), is_prepared(prepared_stmt != nullptr) {}

        /**
         * @brief Destructor - finalizes statement
         */
        ~Statement() {
            if (stmt && is_prepared) {
                sqlite3_finalize(stmt);
            }
        }

        /**
         * @brief Get underlying statement pointer
         *
         * @return sqlite3_stmt* pointer
         */
        sqlite3_stmt* get() const { return stmt; }

        /**
         * @brief Check if statement is valid
         *
         * @return true if statement is prepared and valid
         */
        bool is_valid() const { return is_prepared && stmt != nullptr; }

        /**
         * @brief Step through result rows
         *
         * @return SQLITE_ROW for data row, SQLITE_DONE for end, error code otherwise
         */
        int step() const { return sqlite3_step(stmt); }

        /**
         * @brief Reset statement for reuse
         *
         * @return SQLITE_OK on success
         */
        int reset() const { return sqlite3_reset(stmt); }

        /**
         * @brief Bind integer parameter
         *
         * @param index Parameter index (1-based)
         * @param value Integer value to bind
         * @return SQLITE_OK on success
         */
        int bind_int(int index, int value) const {
            return sqlite3_bind_int(stmt, index, value);
        }

        /**
         * @brief Bind text parameter
         *
         * @param index Parameter index (1-based)
         * @param value Text value to bind
         * @return SQLITE_OK on success
         */
        int bind_text(int index, const std::string& value) const {
            return sqlite3_bind_text(stmt, index, value.c_str(), -1, SQLITE_STATIC);
        }

        /**
         * @brief Bind blob parameter
         *
         * @param index Parameter index (1-based)
         * @param data Pointer to blob data
         * @param size Size of blob data
         * @return SQLITE_OK on success
         */
        int bind_blob(int index, const void* data, int size) const {
            return sqlite3_bind_blob(stmt, index, data, size, SQLITE_STATIC);
        }

        /**
         * @brief Get integer column value
         *
         * @param col_index Column index (0-based)
         * @return Integer value
         */
        int get_int(int col_index) const {
            return sqlite3_column_int(stmt, col_index);
        }

        /**
         * @brief Get text column value
         *
         * @param col_index Column index (0-based)
         * @return Text value as string
         */
        std::string get_text(int col_index) const {
            const unsigned char* text = sqlite3_column_text(stmt, col_index);
            return text ? std::string(reinterpret_cast<const char*>(text)) : "";
        }

        /**
         * @brief Get blob column value
         *
         * @param col_index Column index (0-based)
         * @return Blob data as vector
         */
        std::vector<unsigned char> get_blob(int col_index) const {
            const void* data = sqlite3_column_blob(stmt, col_index);
            int size = sqlite3_column_bytes(stmt, col_index);
            if (!data || size <= 0) {
                return std::vector<unsigned char>();
            }
            return std::vector<unsigned char>(
                static_cast<const unsigned char*>(data),
                static_cast<const unsigned char*>(data) + size
            );
        }

        /**
         * @brief Get number of columns in result
         *
         * @return Number of columns
         */
        int get_column_count() const {
            return sqlite3_column_count(stmt);
        }

        // Prevent copying
        Statement(const Statement&) = delete;
        Statement& operator=(const Statement&) = delete;
    };

    /**
     * @brief Execute SQL statement with no results
     *
     * @param db Database connection
     * @param sql SQL statement to execute
     * @param error_msg Optional error message output
     * @return true if successful
     */
    bool execute_sql(sqlite3* db, const std::string& sql, std::string* error_msg = nullptr);

    /**
     * @brief Execute SQL query and get results
     *
     * @param db Database connection
     * @param sql SQL query to execute
     * @param callback Function called for each result row with column map
     * @param error_msg Optional error message output
     * @return true if successful
     */
    bool execute_query(sqlite3* db, const std::string& sql,
                      std::function<void(const std::map<std::string, std::string>&)> callback,
                      std::string* error_msg = nullptr);

    /**
     * @brief Prepare SQL statement
     *
     * @param db Database connection
     * @param sql SQL statement to prepare
     * @param error_msg Optional error message output
     * @return Statement wrapper, call is_valid() to check success
     */
    Statement prepare_statement(sqlite3* db, const std::string& sql,
                               std::string* error_msg = nullptr);

    /**
     * @brief Get last insert row ID
     *
     * @param db Database connection
     * @return Last inserted row ID
     */
    int get_last_insert_id(sqlite3* db);

    /**
     * @brief Check if table exists
     *
     * @param db Database connection
     * @param table_name Table name to check
     * @return true if table exists
     */
    bool table_exists(sqlite3* db, const std::string& table_name);

    /**
     * @brief Get error message from database
     *
     * @param db Database connection
     * @return Error message string
     */
    std::string get_error_message(sqlite3* db);

} // namespace DatabaseUtils

#endif // DATABASE_UTILS_H
