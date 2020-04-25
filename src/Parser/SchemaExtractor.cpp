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
        create_projections();
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

void SchemaExtractor::create_projections() {
    pugi::xpath_node_set projections = xml_dom.select_nodes("/schema/projections/projection");
    for (pugi::xpath_node p : projections) {
        pugi::xml_node projection = p.node();
        Projection new_projection;
        string projection_name = projection.child_value("projection_name");
        string get_base_table = "/schema/projections/projection[projection_name=\"" + projection_name + "\"]/tables/base_table";
        pugi::xpath_node base_table_xpath_node = xml_dom.select_node(get_base_table.c_str());
        pugi::xml_node base_table_xml_node = base_table_xpath_node.node();
        string base_table = base_table_xml_node.child_value();
        new_projection.set_projection_name(projection_name);
        new_projection.set_base_table(base_table);

        string get_join_tables = "/schema/projections/projection[projection_name=\"" + projection_name + "\"]/tables/join_tables/table";
        pugi::xpath_node_set join_tables = xml_dom.select_nodes(get_join_tables.c_str());
        for (pugi::xpath_node j : join_tables) {
            pugi::xml_node join_table = j.node();
            string from = join_table.child_value("FK_from");
            string fk_table =  join_table.child_value("FK_table");
            string to = join_table.child_value("FK_to");
            
            string get_column_names = "/schema/tables/table[table_name=\"" + base_table + "\"]/columns/column/column_name";
            pugi::xpath_node_set column_names = xml_dom.select_nodes(get_column_names.c_str());
            bool flag1 = false;
            for (pugi::xpath_node c : column_names) {
                pugi::xml_node column_name = c.node();
                if(strcmp(from.c_str(), column_name.child_value()) == 0) {
                    flag1 = true;
                    break;
                }
            }

            if(!flag1) {
                string to_throw = "";
                to_throw += "Attribute ";
                to_throw += from.c_str();
                to_throw += " does not exist in Base Table ";
                to_throw += base_table;
                to_throw += ".";
                throw runtime_error(to_throw);
            }
            
            get_column_names = "/schema/tables/table[table_name=\"" + fk_table + "\"]/columns/column/column_name";
            column_names = xml_dom.select_nodes(get_column_names.c_str());
            bool flag2 = false;
            for (pugi::xpath_node c : column_names) {
                pugi::xml_node column_name = c.node();
                if(strcmp(to.c_str(), column_name.child_value()) == 0) {
                    flag2 = true;
                    break;
                }
            }

            if(!flag2) {
                string to_throw = "";
                to_throw += "Attribute ";
                to_throw += to.c_str();
                to_throw += " does not exist in Join Table ";
                to_throw += fk_table.c_str();
                to_throw += ".";
                throw runtime_error(to_throw);
            }

            new_projection.add_join_table(from, fk_table, to);
        }

        string get_columns = "/schema/projections/projection[projection_name=\"" + projection_name + "\"]/columns/column";
        pugi::xpath_node_set columns = xml_dom.select_nodes(get_columns.c_str());
        for (pugi::xpath_node c : columns) {
            pugi::xml_node column = c.node();
            string name = column.child_value("name");
            string table_name = column.child_value("table_name");
            string column_name = column.child_value("column_name");
            string encoding = column.child_value("encoding");
            if(!encoding.size()) {
                encoding = "default";
            }
            
            string get_column_names = "/schema/tables/table[table_name=\"" + table_name + "\"]/columns/column/column_name";
            pugi::xpath_node_set column_names = xml_dom.select_nodes(get_column_names.c_str());
            bool flag = false;
            for (pugi::xpath_node c : column_names) {
                pugi::xml_node c_name = c.node();
                if(strcmp(column_name.c_str(), c_name.child_value()) == 0) {
                    flag = true;
                    break;
                }
            }

            if(flag) {
                Table table = schema_meta_data.get_table(table_name);
                DataType data_type = table[column_name].type;
                new_projection.add_column(name, table_name, column_name, encoding, data_type);
            } else {
                string to_throw = "";
                to_throw += "Attribute ";
                to_throw += column_name.c_str();
                to_throw += " does not exist in Table ";
                to_throw += table_name.c_str();
                to_throw += ".";
                throw runtime_error(to_throw);
            }
        }

        bool flag = false;
        string get_sort_key = "/schema/projections/projection[projection_name=\"" + projection_name + "\"]/sort_key";
        pugi::xpath_node sort_key_xpath_node = xml_dom.select_node(get_sort_key.c_str());
        pugi::xml_node sort_key_xml_node = sort_key_xpath_node.node();
        string sort_key = sort_key_xml_node.child_value();
        string get_column_names = "/schema/projections/projection[projection_name=\"" + projection_name + "\"]/columns/column/name";
        pugi::xpath_node_set column_names = xml_dom.select_nodes(get_column_names.c_str());
        for (pugi::xpath_node c : column_names) {
            pugi::xml_node column_name = c.node();
            string name = column_name.child_value();
            if(strcmp(sort_key.c_str(), name.c_str()) == 0) {
                flag = true;
                break;
            }
        }

        if(flag) {
            new_projection.set_sort_key(sort_key);
        } else {
            string to_throw = "";
            to_throw += "Attribute ";
            to_throw += sort_key.c_str();
            to_throw += " does not exist in Projection ";
            to_throw += projection_name.c_str();
            to_throw += ".";
            throw runtime_error(to_throw);
        }

        string get_join_indexes = "/schema/projections/projection[projection_name=\"" + projection_name + "\"]/join_indexes/join_index";
        pugi::xpath_node_set join_indexes = xml_dom.select_nodes(get_join_indexes.c_str());
        for (pugi::xpath_node j : join_indexes) {
            pugi::xml_node join_ind = j.node();
            string from = join_ind.child_value("from");
            string other_projection = join_ind.child_value("other_projection");
            string to = join_ind.child_value("to");
            
            string get_column_names = "/schema/projections/projection[projection_name=\"" + projection_name + "\"]/columns/column/name";
            pugi::xpath_node_set column_names = xml_dom.select_nodes(get_column_names.c_str());
            bool flag1 = false;
            for (pugi::xpath_node c : column_names) {
                pugi::xml_node column_name = c.node();
                if(strcmp(from.c_str(), column_name.child_value()) == 0) {
                    flag1 = true;
                    break;
                }
            }

            if(!flag1) {
                string to_throw = "";
                to_throw += "Attribute ";
                to_throw += from.c_str();
                to_throw += " does not exist in Projection ";
                to_throw += projection_name;
                to_throw += ".";
                throw runtime_error(to_throw);
            }
            
            get_column_names = "/schema/projections/projection[projection_name=\"" + other_projection + "\"]/columns/column/name";
            column_names = xml_dom.select_nodes(get_column_names.c_str());
            bool flag2 = false;
            for (pugi::xpath_node c : column_names) {
                pugi::xml_node column_name = c.node();
                if(strcmp(to.c_str(), column_name.child_value()) == 0) {
                    flag2 = true;
                    break;
                }
            }

            if(!flag2) {
                string to_throw = "";
                to_throw += "Attribute ";
                to_throw += to.c_str();
                to_throw += " does not exist in Projection ";
                to_throw += other_projection.c_str();
                to_throw += ".";
                throw runtime_error(to_throw);
            }

            new_projection.add_join_index(from, other_projection, to);
        }

        schema_meta_data.add_projection(new_projection);
    }
}