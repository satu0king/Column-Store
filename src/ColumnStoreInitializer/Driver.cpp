#include <boost/program_options.hpp>

#include "Parser/DataLoader.h"
#include "Parser/SchemaLoader.h"
#include "StoreInitializer.h"

using namespace Parser;

namespace po = boost::program_options;

po::variables_map config;

void initConfig(int ac, char *av[]) {
    try {
        po::options_description desc("Allowed options");
        auto init = desc.add_options();

        init("help", "produce help message");
        init("schema,s",
             po::value<std::string>()->default_value("../schema.xml"),
             "Schema File Path");
        init("db_name,n",
             po::value<std::string>()->default_value("column_store"),
             "Source Database Name");
        init("db_password,p", po::value<std::string>()->default_value("test"),
             "Source Database User Password");
        init("db_user,u", po::value<std::string>()->default_value("test"),
             "Source Database User Password");
        init("seed_db_sql,sql",
             po::value<std::string>()->default_value("../data/dummy_data.sql"),
             "Data for seeding data");
        init("create_tables,ct", po::value<bool>()->default_value(false),
             "Create Tables in Database");
        init("seed_data,sd", po::value<bool>()->default_value(false),
             "Seed Database with data");
        init("column_store_path", po::value<std::string>(),
             "Directory to initialize column store");

        po::store(po::parse_command_line(ac, av, desc), config);
        po::notify(config);

        if (config.count("help")) {
            std::cout << desc << "\n";
            exit(0);
        }

        if (!config.count("column_store_path")) {
            std::cout << "Column store path necessary\n";
            std::cout << desc << "\n";
            exit(EXIT_FAILURE);
        }
    } catch (std::exception &e) {
        std::cerr << "error: " << e.what() << "\n";
        exit(EXIT_FAILURE);
    } catch (...) {
        std::cerr << "Exception of unknown type!\n";
        exit(EXIT_FAILURE);
    }
}

int main(int ac, char *av[]) {
    initConfig(ac, av);

    std::string schema_path = config["schema"].as<std::string>();
    bool create_tables = config["create_tables"].as<bool>();
    bool seed_data = config["seed_data"].as<bool>();

    std::string db_name = config["db_name"].as<std::string>();
    std::string db_user = config["db_user"].as<std::string>();
    std::string db_password = config["db_password"].as<std::string>();

    std::string column_store_path =
        config["column_store_path"].as<std::string>();

    SchemaExtractor schema_extractor(schema_path);

    SchemaMetaData schema_meta_data = schema_extractor.get_meta_data();

    if (create_tables)
        SchemaLoader schema_loader(db_name, db_user, db_password,
                                   schema_meta_data);

    if (seed_data) {
        assert(config.count("seed_db_sql"));
        std::string seed_db_sql = config["seed_db_sql"].as<std::string>();

        DataLoader data_loader(db_name, db_user, db_password, seed_db_sql);
    }

    ColumnStore::Initializer initializer(column_store_path);
    initializer.createProjections(schema_meta_data);
    initializer.configureSourceDB(db_name, db_user, db_password);
    initializer.configureSchema(schema_path);
    initializer.saveMetadata();
    return 0;
}