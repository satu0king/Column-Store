#pragma once 

#include <iostream>
#include <queue>
#include <vector>
#include <pqxx/pqxx>

#include "DataGeneratorInterface.h"
#include "PostgreSQLMetaData.h"
#include "Parser/SchemaMetaData.h"

using namespace std;

class PostgreSQLDataSource : public DataGeneratorInterface {
    std::queue<DataRecord> data;
    pqxx::connection *conn;
    std::string relation_name;
    int offset, batch_size, total_number_of_rows;
    bool relation_is_projection;
    SchemaMetaData schema_meta_data;
    vector<string> columns;

    pqxx::result get_rows_of_columns(pqxx::transaction_base &txn, vector<Column> columns) {
        string sql = "SELECT ";
        for(int i=0; i<columns.size(); i++) {
            string column_name = columns[i].name;
            sql += (column_name + ",");
        }
        sql.pop_back();
        sql += " FROM " + relation_name + " LIMIT " + to_string(batch_size) + " OFFSET " + to_string(offset) + ";";
        // cout << sql << endl;
        offset += batch_size;
        return txn.exec(sql);
    }

    void set_total_number_of_rows(pqxx::transaction_base &txn) {
        string sql = "SELECT count(*) FROM " + relation_name + ";";
        pqxx::result rows = txn.exec(sql);
        total_number_of_rows = rows[0]["count"].as<int>();
    }

    void load_into_queue(pqxx::transaction_base &txn, vector<Column> columns) {
        pqxx::result rows_of_all_columns = get_rows_of_columns(txn, columns);
        for(auto row: rows_of_all_columns) {
            vector<DataValue> values;
            for (int i = 0; i < columns.size(); i++) {
                auto type = columns[i].type;
                string column_name = columns[i].name;
                if (type == DataType::INT) {
                    // cout << (int)row[column_name].as<int>() << "\t";
                    values.push_back(DataValue((int)row[column_name].as<int>()));
                }
                else if (type == DataType::FLOAT) {
                    // cout << (float)row[column_name].as<float>() << "\t";
                    values.push_back(DataValue((float)row[column_name].as<float>()));
                }
                else if (type == DataType::STRING) {
                    // cout << (string)row[column_name].as<string>() << "\t";
                    values.push_back(DataValue((string)row[column_name].as<string>()));
                }
            }
            // cout << endl;
            data.push(values);
        }
    }

    public:
    PostgreSQLDataSource(PostgreSQLMetaData postgresql_meta_data, 
                            std::string r_name, 
                            vector<string> c={}, 
                            int b_size=16) {
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
                Table t = schema_meta_data.get_table(relation_name);
                set_total_number_of_rows(txn);
                vector<Column> metadata_columns;
                if(columns.size() == 0) {
                    metadata_columns = t.get_columns();
                } else {
                    for(int i=0; i<columns.size(); i++) {
                        metadata_columns.push_back(t[columns[i]]);
                    }
                }
                metadata = Metadata(new DataRecordMetadata(metadata_columns));
                load_into_queue(txn, metadata_columns);
            } catch (const TableNotFoundException &e) {
                cout << e.what() << endl;
                vector<Projection> projections = schema_meta_data.get_projections();
                Projection p = schema_meta_data.get_projection(relation_name);
                set_total_number_of_rows(txn);
                vector<Column> metadata_columns;
                if(columns.size() == 0) {
                    vector<projection_column> projection_columns = p.get_columns();
                    for(int i = 0; i < projection_columns.size(); i++) {
                        projection_column p_column = projection_columns[i];
                        Column c = {p_column.name, p_column.data_type, p_column.index};
                        metadata_columns.push_back(c);
                    }
                } else {
                    for(int i=0; i< columns.size(); i++) {
                        projection_column p_column = p[columns[i]];
                        Column c = {p_column.name, p_column.data_type, p_column.index};
                        metadata_columns.push_back(c);
                    }
                }
                metadata = Metadata(new DataRecordMetadata(metadata_columns));
                load_into_queue(txn, metadata_columns);
            } catch (const ProjectionNotFoundException &e) {
                cout << e.what() << endl;
                exit(1);
            }
        } catch (const exception &e) {
            cout << e.what() << endl;
            exit(1);
        }
    }

    DataRecord next() {
        if(data.size() == 0) {
            pqxx::work txn{*conn};
            try {
                try {
                    vector<Table> tables = schema_meta_data.get_tables();
                    Table t = schema_meta_data.get_table(relation_name);
                    vector<Column> metadata_columns;
                    if(columns.size() == 0) {
                        metadata_columns = t.get_columns();
                    } else {
                        for(int i=0; i<columns.size(); i++) {
                            metadata_columns.push_back(t[columns[i]]);
                        }
                    }
                    load_into_queue(txn, metadata_columns);
                } catch (const exception &e) {
                    vector<Projection> projections = schema_meta_data.get_projections();
                    Projection p = schema_meta_data.get_projection(relation_name);
                    vector<projection_column> projection_columns = p.get_columns();
                    vector<Column> metadata_columns;
                    if(columns.size() == 0) {
                        vector<projection_column> projection_columns = p.get_columns();
                        for(int i = 0; i < projection_columns.size(); i++) {
                            projection_column p_column = projection_columns[i];
                            Column c = {p_column.name, p_column.data_type, p_column.index};
                            metadata_columns.push_back(c);
                        }
                    } else {
                        for(int i=0; i< columns.size(); i++) {
                            projection_column p_column = p[columns[i]];
                            Column c = {p_column.name, p_column.data_type, p_column.index};
                            metadata_columns.push_back(c);
                        }
                    }
                    load_into_queue(txn, metadata_columns);
                } 
            } catch (const exception &e) {
                cout << "No relation " + relation_name << endl;
                cerr << e.what() << endl;
                exit(1);
            }
        }
        auto d = data.front();
        data.pop();
        return d;
    }

    bool hasNext() {
        return (offset < total_number_of_rows) || data.size();
    }
};