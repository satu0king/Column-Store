#include "SchemaExtractor.h"

using namespace std;

SchemaExtractor::SchemaExtractor(string xml_fpath) {
    xml_file_path = xml_fpath.c_str();
    pugi::xml_parse_result result = xml_dom.load_file(xml_file_path);
    if(!result) {
        throw runtime_error("XML file not found.");
    }

    try {
        create_tables();
        // create_projections();
    } catch (const exception &e) {
        cerr << e.what() << endl;
        exit(1);
    }
}

void SchemaExtractor::create_tables() {
    pugi::xpath_node_set tables = xml_dom.select_nodes("/schema/tables/table");
    for (pugi::xpath_node t : tables) {
        pugi::xml_node table = t.node();
        Table new_table;
        string table_name = table.child_value("table_name");
        new_table.set_table_name(table_name);
        new_table.set_primary_key(table.child_value("primary_key"));

        string get_columns = "/schema/tables/table[table_name=\"" + table_name + "\"]/columns/column";
        pugi::xpath_node_set columns = xml_dom.select_nodes(get_columns.c_str());
        for (pugi::xpath_node c : columns) {
            pugi::xml_node column = c.node();
            string column_name = column.child_value("column_name");
            string data_type =  column.child_value("data_type");
            new_table.add_column(column_name, data_type);
        }
        
        string get_foreign_keys = "/schema/tables/table[table_name=\"" + table_name + "\"]/foreign_keys/foreign_key";
        pugi::xpath_node_set foreign_keys = xml_dom.select_nodes(get_foreign_keys.c_str());
        for (pugi::xpath_node fk : foreign_keys) {
            pugi::xml_node foreign_key = fk.node();
            string from = foreign_key.child_value("FK_from");
            string fk_table =  foreign_key.child_value("FK_table");
            string to = foreign_key.child_value("FK_to");

            string get_column_names = "/schema/tables/table[table_name=\"" + fk_table + "\"]/columns/column/column_name";
            pugi::xpath_node_set column_names = xml_dom.select_nodes(get_column_names.c_str());
            bool flag = false;
            for (pugi::xpath_node c : column_names) {
                pugi::xml_node column_name = c.node();
                if(strcmp(to.c_str(), column_name.child_value()) == 0) {
                    flag = true;
                    break;
                }
            }

            if(flag) {
                new_table.add_foreign_key(from, fk_table, to);
            } else {
                string to_throw = "Foreign key constraint failed.";
                to_throw += " Attribute ";
                to_throw += to.c_str();
                to_throw += " not in Table ";
                to_throw += fk_table.c_str();
                to_throw += ".";
                throw runtime_error(to_throw);
            }
        }

        schema_meta_data.add_table(new_table);
    }
}