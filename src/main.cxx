
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include <libcellml>

const std::string PARSER_ISSUE_1A = "Given model is a CellML 1.0 model, the parser will try to represent this model in CellML 2.0.";
const std::string PARSER_ISSUE_1B = "Given model is a CellML 1.1 model, the parser will try to represent this model in CellML 2.0.";
const std::string HTML_CONTENT = "Model element is of invalid type 'html'. A valid CellML root node should be of type 'model'.";

const std::vector<std::string> acceptedIssueEndings = {
    "ignoring child element 'documentation'.",
    "ignoring child element 'RDF'.",
    "ignoring child element 'comment'.",
    "ignoring child element 'xml-model'.",
    "ignoring attribute 'rendering_config_file'.",
    "ignoring attribute 'base_units'.",
    "ignoring attribute 'base'.",
    "ignoring child element 'Description'.",
    "ignoring child element 'reaction'.",
};

bool acceptableIssues(libcellml::LoggerPtr logger)
{
    if (logger->issue(0)->description() != PARSER_ISSUE_1A &&
            logger->issue(0)->description() != PARSER_ISSUE_1B) {
        return false;
    }

    for (size_t i = 1; i < logger->issueCount(); ++i) {
        bool acceptable = logger->issue(i)->level() == libcellml::Issue::Level::MESSAGE;
//        if (logger->issue(i)->level() == libcellml::Issue::Level::MESSAGE) {

//        }
//        const std::string description = logger->issue(i)->description();
//        for(const auto& value: acceptedIssueEndings) {
//            if (description.length() > value.length() && 0 == description.compare (description.length() - value.length(), value.length(), value)) {
//                acceptable = true;
//            }
//        }
        if (!acceptable) {
            return false;
        }
    }

    return true;
}

bool htmlContentIssue(libcellml::LoggerPtr logger)
{
    if (logger->issueCount() == 1) {
        if (logger->issue(0)->description() == HTML_CONTENT) {
            return true;
        }
    }

    return false;
}

int main(int argc, char *argv[])
{
    auto p = libcellml::Parser::create(false);
    auto v = libcellml::Validator::create();

    std::vector<std::string> args;
    std::copy(argv + 1, argv + argc, std::back_inserter(args));

    int return_code = 9;
    if (args.size() == 1) {
        return_code = 1;
        std::ifstream ifs(args.at(0));
        std::string content( (std::istreambuf_iterator<char>(ifs) ),
                               (std::istreambuf_iterator<char>()    ) );
        auto m = p->parseModel(content);
        if (acceptableIssues(p)) {
            return_code = 2;
            v->validateModel(m);
            if (v->issueCount() == 0) {
                return_code = 0;
            }
        } else {
            if (htmlContentIssue(p)) {
                return_code = 4;
            } else {
                std::cout << "====" << std::endl;
                std::cout << args.at(0) << std::endl;
                for (size_t i = 0; i < p->issueCount(); ++i) {
                    if (p->issue(i)->level() != libcellml::Issue::Level::MESSAGE) {
                        std::cout << i << ": " << p->issue(i)->description() << std::endl;
                    }
                }
            }
        }
    }

    return return_code;
}
