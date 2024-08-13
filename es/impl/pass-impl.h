#ifndef ES_IMPL_PASS_H
#define ES_IMPL_PASS_H

#include <es/pass.h>
#include <es/parser/ast.h>

namespace es {

AST* Optimize(AST* ast) {
  if (ast == nullptr) {
    return nullptr;
  }
  switch (ast->type()) {
    case AST::AST_PROGRAM:
    case AST::AST_FUNC_BODY: {
      auto prog = static_cast<ProgramOrFunctionBody*>(ast);
      for (size_t i = 0; i < prog->statements().size(); i++) {
        prog->stmts_[i] = Optimize(prog->statements()[i]);
      }
      for (size_t i = 0; i < prog->func_decls_.size(); i++) {
        prog->func_decls_[i] = static_cast<Function*>(Optimize(prog->func_decls_[i]));
      }
      break;
    }
    case AST::AST_FUNC: {
      auto func = static_cast<Function*>(ast);
      func->body_ = static_cast<ProgramOrFunctionBody*>(Optimize(func->body()));
      break;
    }
    case AST::AST_STMT_BLOCK: {
      auto block = static_cast<Block*>(ast);
      for (size_t i = 0; i < block->statements().size(); i++) {
        block->stmts_[i] = Optimize(block->statements()[i]);
      }
      break;
    }
    case AST::AST_STMT_VAR: {
      auto var = static_cast<VarStmt*>(ast);
      for (size_t i = 0; i < var->decls().size(); i++) {
        var->decls_[i] = static_cast<VarDecl*>(Optimize(var->decls()[i]));
      }
      break;
    }
    case AST::AST_STMT_VAR_DECL: {
      auto decl = static_cast<VarDecl*>(ast);
      if (decl->init()) {
        decl->init_ = Optimize(decl->init());
      }
      break;
    }
    case AST::AST_STMT_IF: {
      auto if_stmt = static_cast<If*>(ast);
      if_stmt->cond_ = Optimize(if_stmt->cond());
      if_stmt->if_block_ = Optimize(if_stmt->if_block());
      if (if_stmt->else_block() != nullptr) {
        if_stmt->else_block_ = Optimize(if_stmt->else_block());
      }
      break;
    }
    case AST::AST_STMT_DO_WHILE: {
      auto do_while = static_cast<DoWhile*>(ast);
      do_while->expr_ = Optimize(do_while->expr());
      do_while->stmt_ = Optimize(do_while->stmt());
      break;
    }
    case AST::AST_STMT_WHILE:
    case AST::AST_STMT_WITH: {
      auto while_stmt = static_cast<WhileOrWith*>(ast);
      while_stmt->expr_ = Optimize(while_stmt->expr());
      while_stmt->stmt_ = Optimize(while_stmt->stmt());
      break;
    }
    case AST::AST_STMT_FOR: {
      auto for_stmt = static_cast<For*>(ast);
      for (size_t i = 0; i < for_stmt->expr0s_.size(); i++) {
        for_stmt->expr0s_[i] = Optimize(for_stmt->expr0s_[i]);
      }
      for_stmt->expr1_ = Optimize(for_stmt->expr1());
      for_stmt->expr2_ = Optimize(for_stmt->expr2());
      break;
    }
    case AST::AST_STMT_FOR_IN: {
      auto for_in = static_cast<ForIn*>(ast);
      for_in->expr0_ = Optimize(for_in->expr0_);
      for_in->expr1_ = Optimize(for_in->expr1_);
      for_in->stmt_ = Optimize(for_in->stmt_);
      break;
    }
    case AST::AST_STMT_RETURN: {
      auto ret = static_cast<Return*>(ast);
      if (ret->expr() != nullptr) {
        ret->expr_ = Optimize(ret->expr());
      }
      break;
    }
    case AST::AST_STMT_THROW: {
      auto throw_stmt = static_cast<Throw*>(ast);
      throw_stmt->expr_ = Optimize(throw_stmt->expr());
      break;
    }
    case AST::AST_STMT_LABEL: {
      auto label = static_cast<LabelledStmt*>(ast);
      label->stmt_ = Optimize(label->statement());
      break;
    }

    /* Expression */
    case AST::AST_EXPR_LHS: {
      auto lhs = static_cast<LHS*>(ast);
      AST* base = Optimize(lhs->base());
      if (lhs->total_count() == 0) {
        //std::cout << "total_count == 0 " << log::ToString(base->source().substr(0, 30)) << std::endl;
        return base;
      }
      lhs->base_ = base;
      for (size_t i = 0; i < lhs->args_list_.size(); i++) {
        lhs->args_list_[i] = static_cast<Arguments*>(Optimize(lhs->args_list_[i]));
      }
      for (size_t i = 0; i < lhs->index_list_.size(); i++) {
        lhs->index_list_[i] = Optimize(lhs->index_list_[i]);
      }
      break;
    }
    case AST::AST_EXPR_ARGS: {
      auto args = static_cast<Arguments*>(ast);
      for (size_t i = 0; i < args->args().size(); i++) {
        args->args_[i] = Optimize(args->args()[i]);
      }
      break;
    }
    case AST::AST_EXPR_OBJ: {
      auto obj = static_cast<ObjectLiteral*>(ast);
      for (size_t i = 0; i < obj->properties().size(); i++) {
        obj->properties_[i].value = Optimize(obj->properties_[i].value);
      }
      break;
    }
    case AST::AST_EXPR_ARRAY: {
      auto arr = static_cast<ArrayLiteral*>(ast);
      for (size_t i = 0; i < arr->elements().size(); i++) {
        arr->elements_[i] = Optimize(arr->elements_[i]);
      }
      break;
    }
    case AST::AST_EXPR_UNARY: {
      auto unary = static_cast<Unary*>(ast);
      unary->node_ = Optimize(unary->node_);
      break;
    }
    case AST::AST_EXPR_BINARY: {
      auto binary = static_cast<Binary*>(ast);
      binary->lhs_ = Optimize(binary->lhs());
      binary->rhs_ = Optimize(binary->rhs());
      break;
    }
    case AST::AST_EXPR_TRIPLE: {
      auto triple = static_cast<TripleCondition*>(ast);
      triple->cond_ = Optimize(triple->cond());
      triple->true_expr_ = Optimize(triple->true_expr());
      triple->false_expr_ = Optimize(triple->false_expr());
      break;
    }
    case AST::AST_EXPR_PAREN: {
      auto paren = static_cast<Paren*>(ast);
      return Optimize(paren->expr());
    }
    case AST::AST_EXPR: {
      auto expr = static_cast<Expression*>(ast);
      for (size_t i = 0; i < expr->elements_.size(); i++) {
        expr->elements_[i] = Optimize(expr->elements_[i]);
      }
      break;
    }

    /* Do Nothing */
    // stmt
    case AST::AST_STMT_DEBUG:
    case AST::AST_STMT_EMPTY:
    case AST::AST_STMT_CONTINUE:
    case AST::AST_STMT_BREAK:
      [[fallthrough]];
    // expr
    case AST::AST_EXPR_STRICT_FUTURE:
    case AST::AST_EXPR_THIS:
    case AST::AST_EXPR_IDENT:
    case AST::AST_EXPR_NULL:
    case AST::AST_EXPR_BOOL:
    case AST::AST_EXPR_NUMBER:
    case AST::AST_EXPR_STRING:
    case AST::AST_EXPR_REGEXP:
      [[fallthrough]];
    /* TODOs */
    // stmt
    case AST::AST_STMT_SWITCH:
    case AST::AST_STMT_TRY:
    // expr
      break;
    default:
      std::cout << "Unknown AST type: " << ast->type() << " " << log::ToString(ast->source().substr(0, 30)) << std::endl;
      break;
  }

  return ast;
}

}  // namespace es

#endif  // ES_IMPL_PASS_H