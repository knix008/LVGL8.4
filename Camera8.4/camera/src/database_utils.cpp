#include "database_utils.h"
#include <iostream>

namespace DatabaseUtils {

bool execute_sql(sqlite3* db, const std::string& sql, std::string* error_msg) {
    if (!db) {
        if (error_msg) *error_msg = "Database connection is null";
        return false;
    }

    char* error = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &error);

    if (rc != SQLITE_OK) {
        if (error_msg) {
            *error_msg = error ? std::string(error) : "Unknown SQL error";
        }
        if (error) {
            sqlite3_free(error);
        }
        return false;
    }

    return true;
}

bool execute_query(sqlite3* db, const std::string& sql,
                  std::function<void(const std::map<std::string, std::string>&)> callback,
                  std::string* error_msg) {
    if (!db) {
        if (error_msg) *error_msg = "Database connection is null";
        return false;
    }

    Statement stmt = prepare_statement(db, sql, error_msg);
    if (!stmt.is_valid()) {
        return false;
    }

    while (stmt.step() == SQLITE_ROW) {
        std::map<std::string, std::string> row;

        int col_count = stmt.get_column_count();
        for (int i = 0; i < col_count; ++i) {
            const char* col_name = sqlite3_column_name(stmt.get(), i);
            std::string value = stmt.get_text(i);
            if (col_name) {
                row[col_name] = value;
            }
        }

        callback(row);
    }

    return true;
}

Statement prepare_statement(sqlite3* db, const std::string& sql, std::string* error_msg) {
    if (!db) {
        if (error_msg) *error_msg = "Database connection is null";
        return Statement(nullptr);
    }

    sqlite3_stmt* stmt = nullptr;
    const char* tail = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, &tail);

    if (rc != SQLITE_OK) {
        if (error_msg) {
            *error_msg = sqlite3_errmsg(db);
        }
        return Statement(nullptr);
    }

    return Statement(stmt);
}

int get_last_insert_id(sqlite3* db) {
    if (!db) return -1;
    return static_cast<int>(sqlite3_last_insert_rowid(db));
}

bool table_exists(sqlite3* db, const std::string& table_name) {
    if (!db) return false;

    std::string sql = "SELECT name FROM sqlite_master WHERE type='table' AND name=?;";
    Statement stmt = prepare_statement(db, sql);

    if (!stmt.is_valid()) {
        return false;
    }

    stmt.bind_text(1, table_name);

    if (stmt.step() == SQLITE_ROW) {
        return true;
    }

    return false;
}

std::string get_error_message(sqlite3* db) {
    if (!db) return "Database connection is null";
    return sqlite3_errmsg(db);
}

} // namespace DatabaseUtils
