#pragma once

#include <iostream>
#include <pqxx/pqxx>
#include <queue>
#include <vector>

#include "interfaces/DataGeneratorInterface.h"
#include "Parser/SchemaMetaData.h"
#include "PostgreSQLMetaData.h"

using ColumnStore::DataRecord;
using ColumnStore::DataRecordMetadata;
using ColumnStore::DataSource;
using ColumnStore::DataType;
using ColumnStore::DataValue;
using ColumnStore::Metadata;
using Parser::Projection;
using Parser::projection_column;
using Parser::Table;
using std::vector;

namespace Postgres {
class PostgreSQLDataSource : public ColumnStore::DataGeneratorInterface {
    std::queue<DataRecord> data;
    pqxx::connection *conn;
    std::string relation_name;
    int offset, batch_size, total_number_of_rows;
    SchemaMetaData schema_meta_data;
    vector<std::string> columns;

    pqxx::result get_rows_of_columns(pqxx::transaction_base &txn,
                                     vector<ColumnStore::Column> columns) {
        std::string sql = "SELECT ";
        for (int i = 0; i < int(columns.size()); i++) {
            std::string column_name = columns[i].name;
            sql += (column_name + ",");
        }
        sql.pop_back();
        sql += " FROM " + relation_name + " LIMIT " + std::to_string(batch_size) +
               " OFFSET " + std::to_string(offset) + ";";
        offset += batch_size;
        return txn.exec(sql);
    }

    void set_total_number_of_rows(pqxx::transaction_base &txn) {
        std::string sql = "SELECT count(*) FROM " + relation_name + ";";
        pqxx::result rows = txn.exec(sql);
        total_number_of_rows = rows[0]["count"].as<int>();
    }

    void load_into_queue(pqxx::transaction_base &txn,
                         vector<ColumnStore::Column> columns) {
        pqxx::result rows_of_all_columns = get_rows_of_columns(txn, columns);
        for (auto row : rows_of_all_columns) {
            vector<DataValue> values;
            for (int i = 0; i < int(columns.size()); i++) {
                auto type = columns[i].type;
                std::string column_name = columns[i].name;
                if (type == DataType::INT) {
                    values.push_back(
                        DataValue((int)row[column_name].as<int>()));
                } else if (type == DataType::FLOAT) {
                    values.push_back(
                        DataValue((float)row[column_name].as<float>()));
                } else if (type == DataType::STRING) {
                    values.push_back(
                        DataValue((std::string)row[column_name].as<std::string>()));
                }
            }
            data.push(values);
        }
    }

   public:
    PostgreSQLDataSource(PostgreSQLMetaData postgresql_meta_data,
                         std::string r_name, vector<std::string> c = {},
                         int b_size = 16) {
        conn = postgresql_meta_data.get_connection();
        pqxx::work txn{*conn};
        relation_name = r_name;
        columns = c;
        batch_size = b_size;
        offset = 0;
        schema_meta_data = postgresql_meta_data.get_schema_meta_data();
        try {
            try {
                vector<Table> tables = schema_meta_data.get_tables();
                Table table = schema_meta_data.get_table(relation_name);
                set_total_number_of_rows(txn);
                vector<ColumnStore::Column> metadata_columns;
                if (int(columns.size()) == 0) {
                    auto temp = table.get_columns();
                    for (auto column : temp) metadata_columns.push_back(column);
                } else {
                    for (int i = 0; i < int(columns.size()); i++) {
                        metadata_columns.push_back(table[columns[i]]);
                    }
                }
                metadata = Metadata(new DataRecordMetadata(metadata_columns));
                load_into_queue(txn, metadata_columns);
            } catch (const Parser::TableNotFoundException &e) {
                vector<Projection> projections =
                    schema_meta_data.get_projections();
                Projection p = schema_meta_data.get_projection(relation_name);
                set_total_number_of_rows(txn);
                vector<ColumnStore::Column> metadata_columns;
                if (int(columns.size()) == 0) {
                    vector<projection_column> projection_columns =
                        p.get_columns();
                    for (int i = 0; i < int(projection_columns.size()); i++) {
                        projection_column p_column = projection_columns[i];
                        ColumnStore::Column c = {
                            p_column.name, p_column.data_type, p_column.index};
                        metadata_columns.push_back(c);
                    }
                } else {
                    for (int i = 0; i < int(columns.size()); i++) {
                        projection_column p_column = p[columns[i]];
                        ColumnStore::Column c = {
                            p_column.name, p_column.data_type, p_column.index};
                        metadata_columns.push_back(c);
                    }
                }
                metadata = Metadata(new DataRecordMetadata(metadata_columns));
                load_into_queue(txn, metadata_columns);
            } catch (const Parser::ProjectionNotFoundException &e) {
                std::cout << e.what() << std::endl;
                exit(1);
            }
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            exit(1);
        }
    }

    DataRecord next() {
        if (data.size() == 0) {
            pqxx::work txn{*conn};
            try {
                try {
                    vector<Table> tables = schema_meta_data.get_tables();
                    Table table = schema_meta_data.get_table(relation_name);
                    vector<ColumnStore::Column> metadata_columns;
                    if (int(columns.size()) == 0) {
                        for (auto c : table.get_columns())
                            metadata_columns.push_back(c);
                    } else {
                        for (int i = 0; i < int(columns.size()); i++) {
                            metadata_columns.push_back(table[columns[i]]);
                        }
                    }
                    load_into_queue(txn, metadata_columns);
                } catch (const std::exception &e) {
                    vector<Projection> projections =
                        schema_meta_data.get_projections();
                    Projection p =
                        schema_meta_data.get_projection(relation_name);
                    vector<projection_column> projection_columns =
                        p.get_columns();
                    vector<ColumnStore::Column> metadata_columns;
                    if (int(columns.size()) == 0) {
                        vector<projection_column> projection_columns =
                            p.get_columns();
                        for (int i = 0; i < int(projection_columns.size()); i++) {
                            projection_column p_column = projection_columns[i];
                            ColumnStore::Column c = {p_column.name,
                                                     p_column.data_type,
                                                     p_column.index};
                            metadata_columns.push_back(c);
                        }
                    } else {
                        for (int i = 0; i < int(columns.size()); i++) {
                            projection_column p_column = p[columns[i]];
                            ColumnStore::Column c = {p_column.name,
                                                     p_column.data_type,
                                                     p_column.index};
                            metadata_columns.push_back(c);
                        }
                    }
                    load_into_queue(txn, metadata_columns);
                }
            } catch (const std::exception &e) {
                std::cout << "No relation " + relation_name << std::endl;
                std::cerr << e.what() << std::endl;
                exit(1);
            }
        }
        auto d = data.front();
        data.pop();
        return d;
    }

    void advance(int recordCount) {
        while(data.size() && recordCount) {
            recordCount--;
            data.pop();
        }
        if(recordCount) {
            offset += recordCount;
        }
    }

    bool hasNext() { return (offset < total_number_of_rows) || data.size(); }
};
}