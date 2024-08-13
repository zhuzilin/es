#ifndef ES_PASS_H
#define ES_PASS_H

namespace es {

class AST;
AST* Optimize(AST* ast);

}  // namespace es

#endif  // ES_PASS_H