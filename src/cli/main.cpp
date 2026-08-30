#include "kyma/kyma.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace kyma;
static int runSource(const std::string& source,const std::string& name,Interpreter& interpreter,bool checkOnly=false){try{auto program=Parser(lex(source)).parse();auto errors=Analyzer().analyze(program);for(const auto&d:errors)std::cerr<<formatDiagnostic(d,name)<<'\n';if(std::any_of(errors.begin(),errors.end(),[](const auto&d){return !d.warning;}))return 1;if(!checkOnly)interpreter.execute(program);return 0;}catch(const KymaError&e){std::cerr<<formatDiagnostic(e.diagnostic,name)<<'\n';return 1;}catch(const std::exception&e){std::cerr<<name<<": runtime error: "<<e.what()<<'\n';return 1;}}
static int repl(){Interpreter i;std::string line,source;int braces=0;std::cout<<"Kyma 0.1 REPL (type :help for help, :quit to exit)\n"<<">> ";while(std::getline(std::cin,line)){if(line==":quit"||line==":q")break;if(line==":help"){std::cout<<"Enter Kyma statements; blocks may span lines. :quit exits.\n>> ";continue;}source+=line+'\n';for(char c:line)braces+=(c=='{')-(c=='}');if(braces<=0&&!source.empty()){runSource(source,"<repl>",i);source.clear();braces=0;}std::cout<<">> ";}return 0;}
int main(int argc,char**argv){if(argc==1)return repl();bool check=false;std::string file;for(int n=1;n<argc;++n){std::string a=argv[n];if(a=="--check")check=true;else if(a=="--repl")return repl();else if(a=="--help"){std::cout<<"Usage: kyma [--check] file.kyma | --repl\n";return 0;}else file=a;}if(file.empty()){std::cerr<<"no input file\n";return 2;}std::ifstream in(file);if(!in){std::cerr<<"cannot open '"<<file<<"'\n";return 2;}std::stringstream ss;ss<<in.rdbuf();Interpreter i;return runSource(ss.str(),file,i,check);}
