#ifndef ES_EVALUATOR_H
#define ES_EVALUATOR_H

#include <math.h>

#include <es/parser/character.h>
#include <es/parser/ast.h>
#include <es/types/completion.h>
#include <es/types/reference.h>
#include <es/types/builtin/function_object.h>
#include <es/types/builtin/array_object.h>
#include <es/types/builtin/error_object.h>
#include <es/types/compare.h>
#include <es/execution_context.h>
#include <es/utils/helper.h>

namespace es {

Completion EvalProgram(AST* ast);

Completion EvalStatement(AST* ast);
Completion EvalStatementList(std::vector<AST*> statements);
Completion EvalBlockStatement(AST* ast);
std::u16string EvalVarDecl(Error* e, AST* ast);
Completion EvalVarStatement(AST* ast);
Completion EvalIfStatement(AST* ast);
Completion EvalForStatement(AST* ast);
Completion EvalForInStatement(AST* ast);
Completion EvalDoWhileStatement(AST* ast);
Completion EvalWhileStatement(AST* ast);
Completion EvalContinueStatement(AST* ast);
Completion EvalBreakStatement(AST* ast);
Completion EvalReturnStatement(AST* ast);
Completion EvalLabelledStatement(AST* ast);
Completion EvalSwitchStatement(AST* ast);
Completion EvalThrowStatement(AST* ast);
Completion EvalTryStatement(AST* ast);
Completion EvalExpressionStatement(AST* ast);

JSValue* EvalExpression(Error* e, AST* ast);
JSValue* EvalPrimaryExpression(Error* e, AST* ast);
Reference* EvalIdentifier(AST* ast);
Number* EvalNumber(AST* ast);
String* EvalString(AST* ast);
Object* EvalObject(Error* e, AST* ast);
ArrayObject* EvalArray(Error* e, AST* ast);
JSValue* EvalUnaryOperator(Error* e, AST* ast);
JSValue* EvalBinaryExpression(Error* e, AST* ast);
JSValue* EvalBinaryExpression(Error* e, std::u16string op, AST* lval, AST* rval);
JSValue* EvalBinaryExpression(Error* e, std::u16string op, JSValue* lval, JSValue* rval);
JSValue* EvalArithmeticOperator(Error* e, std::u16string op, JSValue* lval, JSValue* rval);
JSValue* EvalAddOperator(Error* e, JSValue* lval, JSValue* rval);
JSValue* EvalBitwiseShiftOperator(Error* e, std::u16string op, JSValue* lval, JSValue* rval);
JSValue* EvalRelationalOperator(Error* e, std::u16string op, JSValue* lval, JSValue* rval);
JSValue* EvalEqualityOperator(Error* e, std::u16string op, JSValue* lval, JSValue* rval);
JSValue* EvalBitwiseOperator(Error* e, std::u16string op, JSValue* lval, JSValue* rval);
JSValue* EvalLogicalOperator(Error* e, std::u16string op, AST* lhs, AST* rhs);
JSValue* EvalSimpleAssignment(Error* e, JSValue* lref, JSValue* rval);
JSValue* EvalCompoundAssignment(Error* e, std::u16string op, JSValue* lref, JSValue* rval);
JSValue* EvalTripleConditionExpression(Error* e, AST* ast);
JSValue* EvalAssignmentExpression(Error* e, AST* ast);
JSValue* EvalLeftHandSideExpression(Error* e, AST* ast);
std::vector<JSValue*> EvalArgumentsList(Error* e, Arguments* ast);
JSValue* EvalCallExpression(Error* e, JSValue* ref, std::vector<JSValue*> arg_list);
JSValue* EvalIndexExpression(Error* e, JSValue* base_ref, std::u16string identifier_name, ValueGuard& guard);
JSValue* EvalIndexExpression(Error* e, JSValue* base_ref, AST* expr, ValueGuard& guard);
JSValue* EvalExpressionList(Error* e, AST* ast);

Reference* IdentifierResolution(std::u16string name);

Completion EvalProgram(AST* ast) {
  assert(ast->type() == AST::AST_PROGRAM || ast->type() == AST::AST_FUNC_BODY);
  auto prog = static_cast<ProgramOrFunctionBody*>(ast);
  auto statements = prog->statements();
  // 12.9 considered syntactically incorrect if it contains
  //      a return statement that is not within a FunctionBody.
  if (ast->type() != AST::AST_FUNC_BODY) {
    for (auto stmt : statements) {
      if (stmt->type() == AST::AST_STMT_RETURN) {
        return Completion(Completion::THROW, new ErrorObject(Error::SyntaxError()), u"");
      }
    }
  }

  Completion head_result;
  if (statements.size() == 0)
    return Completion(Completion::NORMAL, nullptr, u"");
  for (auto stmt : prog->statements()) {
    if (head_result.IsAbruptCompletion())
      break;
    Completion tail_result = EvalStatement(stmt);
    if (tail_result.IsThrow())
      return tail_result;
    head_result = Completion(
      tail_result.type,
      tail_result.value == nullptr? head_result.value : tail_result.value,
      tail_result.target
    );
  }
  return head_result;
}

Completion EvalStatement(AST* ast) {
  switch(ast->type()) {
    case AST::AST_STMT_BLOCK:
      return EvalBlockStatement(ast);
    case AST::AST_STMT_VAR:
      return EvalVarStatement(ast);
    case AST::AST_STMT_EMPTY:
      return Completion(Completion::NORMAL, nullptr, u"");
    case AST::AST_STMT_IF:
      return EvalIfStatement(ast);
    case AST::AST_STMT_DO_WHILE:
      return EvalDoWhileStatement(ast);
    case AST::AST_STMT_WHILE:
      return EvalWhileStatement(ast);
    case AST::AST_STMT_FOR:
      return EvalForStatement(ast);
    case AST::AST_STMT_FOR_IN:
      return EvalForInStatement(ast);
    case AST::AST_STMT_CONTINUE:
      return EvalContinueStatement(ast);
    case AST::AST_STMT_BREAK:
      return EvalBreakStatement(ast);
    case AST::AST_STMT_RETURN:
      return EvalReturnStatement(ast);
    case AST::AST_STMT_WITH:
      assert(false);
    case AST::AST_STMT_LABEL:
      return EvalLabelledStatement(ast);
    case AST::AST_STMT_SWITCH:
      return EvalSwitchStatement(ast);
    case AST::AST_STMT_THROW:
      return EvalThrowStatement(ast);
    case AST::AST_STMT_TRY:
      return EvalTryStatement(ast);
    case AST::AST_STMT_DEBUG:
      return Completion(Completion::NORMAL, nullptr, u"");
    default:
      return EvalExpressionStatement(ast);
  }
}

Completion EvalStatementList(std::vector<AST*> statements) {
  Completion sl;
  for (auto stmt : statements) {
    Completion s = EvalStatement(stmt);
    if (s.IsThrow())
      return s;
    sl = Completion(s.type, s.value == nullptr ? sl.value : s.value, s.target);
    if (sl.IsAbruptCompletion())
      return sl;
  }
  return sl;
}

Completion EvalBlockStatement(AST* ast) {
  assert(ast->type() == AST::AST_STMT_BLOCK);
  Block* block = static_cast<Block*>(ast);
  return EvalStatementList(block->statements());
}

std::u16string EvalVarDecl(Error* e, AST* ast) {
  assert(ast->type() == AST::AST_STMT_VAR_DECL);
  VarDecl* decl = static_cast<VarDecl*>(ast);
  if (decl->init() == nullptr)
    return decl->ident();
  JSValue* lhs = IdentifierResolution(decl->ident());
  JSValue* rhs = EvalAssignmentExpression(e, decl->init());
  if (!e->IsOk()) return decl->ident();
  JSValue* value = GetValue(e, rhs);
  if (!e->IsOk()) return decl->ident();
  PutValue(e, lhs, value);
  if (!e->IsOk()) return decl->ident();
  return decl->ident();
}

Completion EvalVarStatement(AST* ast) {
  assert(ast->type() == AST::AST_STMT_VAR);
  Error* e = Error::Ok();
  VarStmt* var_stmt = static_cast<VarStmt*>(ast);
  for (VarDecl* decl : var_stmt->decls()) {
    if (decl->init() == nullptr)
      continue;
    EvalVarDecl(e, decl);
    if (!e->IsOk()) goto error;
  }
  return Completion(Completion::NORMAL, nullptr, u"");
error:
  return Completion(Completion::THROW, new ErrorObject(e), u"");
}

Completion EvalIfStatement(AST* ast) {
  assert(ast->type() == AST::AST_STMT_IF);
  Error* e = Error::Ok();
  If* if_stmt = static_cast<If*>(ast);
  JSValue* expr_ref = EvalExpression(e, if_stmt->cond());
  if (!e->IsOk())
    return Completion(Completion::THROW, new ErrorObject(e), u"");
  JSValue* expr = GetValue(e, expr_ref);
  if (!e->IsOk())
    return Completion(Completion::THROW, new ErrorObject(e), u"");
  if (ToBoolean(expr)) {
    return EvalStatement(if_stmt->if_block());
  } else if (if_stmt->else_block() != nullptr){
    return EvalStatement(if_stmt->else_block());
  }
  return Completion(Completion::NORMAL, nullptr, u"");
}

// 12.6.1 The do-while Statement
Completion EvalDoWhileStatement(AST* ast) {
  assert(ast->type() == AST::AST_STMT_DO_WHILE);
  Error* e = Error::Ok();
  RuntimeContext::TopContext()->EnterIteration();
  DoWhile* loop_stmt = static_cast<DoWhile*>(ast);
  JSValue* V = nullptr;
  JSValue* expr_ref;
  JSValue* val;
  Completion stmt;
  bool has_label;
  while (true) {
    stmt = EvalStatement(loop_stmt->stmt());
    if (stmt.value != nullptr)  // 3.b
      V = stmt.value;
    has_label = stmt.target == ast->label() || stmt.target == u"";
    if (stmt.type != Completion::CONTINUE || !has_label) {
      if (stmt.type == Completion::BREAK && has_label) {
        RuntimeContext::TopContext()->ExitIteration();
        return Completion(Completion::NORMAL, V, u"");
      }
      if (stmt.IsAbruptCompletion()) {
        RuntimeContext::TopContext()->ExitIteration();
        return stmt;
      }
    }

    expr_ref = EvalExpression(e, loop_stmt->expr());
    if (!e->IsOk()) goto error;
    val = GetValue(e, expr_ref);
    if (!e->IsOk()) goto error;
    if (!ToBoolean(val))
      break;
  }
  RuntimeContext::TopContext()->ExitIteration();
  return Completion(Completion::NORMAL, V, u"");
error:
  RuntimeContext::TopContext()->ExitIteration();
  return Completion(Completion::THROW, new ErrorObject(e), u"");
}

// 12.6.2 The while Statement
Completion EvalWhileStatement(AST* ast) {
  assert(ast->type() == AST::AST_STMT_WHILE);
  Error* e = Error::Ok();
  RuntimeContext::TopContext()->EnterIteration();
  WhileOrWith* loop_stmt = static_cast<WhileOrWith*>(ast);
  JSValue* V = nullptr;
  JSValue* expr_ref;
  JSValue* val;
  Completion stmt;
  bool has_label;
  while (true) {
    expr_ref = EvalExpression(e, loop_stmt->expr());
    if (!e->IsOk()) goto error;
    val = GetValue(e, expr_ref);
    if (!e->IsOk()) goto error;
    if (!ToBoolean(val))
      break;

    stmt = EvalStatement(loop_stmt->stmt());
    if (stmt.value != nullptr)  // 3.b
      V = stmt.value;
    has_label = stmt.target == ast->label() || stmt.target == u"";
    if (stmt.type != Completion::CONTINUE || !has_label) {
      if (stmt.type == Completion::BREAK && has_label) {
        RuntimeContext::TopContext()->ExitIteration();
        return Completion(Completion::NORMAL, V, u"");
      }
      if (stmt.IsAbruptCompletion()) {
        RuntimeContext::TopContext()->ExitIteration();
        return stmt;
      }
    }
  }
  RuntimeContext::TopContext()->ExitIteration();
  return Completion(Completion::NORMAL, V, u"");
error:
  RuntimeContext::TopContext()->ExitIteration();
  return Completion(Completion::THROW, new ErrorObject(e), u"");
}

// 12.6.3 The for Statement
Completion EvalForStatement(AST* ast) {
  assert(ast->type() == AST::AST_STMT_FOR);
  Error* e = Error::Ok();
  RuntimeContext::TopContext()->EnterIteration();
  For* for_stmt = static_cast<For*>(ast);
  JSValue* V = nullptr;
  Completion stmt;
  bool has_label;
  for (auto expr : for_stmt->expr0s()) {
    if (expr->type() == AST::AST_STMT_VAR_DECL) {
      EvalVarDecl(e, expr);
      if (!e->IsOk()) goto error;
    } else {
      JSValue* expr_ref = EvalExpression(e, expr);
      if (!e->IsOk()) goto error;
      GetValue(e, expr_ref);
      if (!e->IsOk()) goto error;
    }
  }
  while (true) {
    if (for_stmt->expr1() != nullptr) {
      JSValue* test_expr_ref = EvalExpression(e, for_stmt->expr1());
      if (!e->IsOk()) goto error;
      JSValue* test_value = GetValue(e, test_expr_ref);
      if (!e->IsOk()) goto error;
      if (!ToBoolean(test_value))
        break;
    }

    stmt = EvalStatement(for_stmt->statement());
    if (stmt.value != nullptr)  // 3.b
      V = stmt.value;
    has_label = stmt.target == ast->label() || stmt.target == u"";
    if (stmt.type != Completion::CONTINUE || !has_label) {
      if (stmt.type == Completion::BREAK && has_label) {
        RuntimeContext::TopContext()->ExitIteration();
        return Completion(Completion::NORMAL, V, u"");
      }
      if (stmt.IsAbruptCompletion()) {
        RuntimeContext::TopContext()->ExitIteration();
        return stmt;
      }
    }

    if (for_stmt->expr2() != nullptr) {
      JSValue* inc_expr_ref = EvalExpression(e, for_stmt->expr2());
      if (!e->IsOk()) goto error;
      GetValue(e, inc_expr_ref);
      if (!e->IsOk()) goto error;
    }
  }
  RuntimeContext::TopContext()->ExitIteration();
  return Completion(Completion::NORMAL, V, u"");
error:
  RuntimeContext::TopContext()->ExitIteration();
  return Completion(Completion::THROW, new ErrorObject(e), u"");
}

// 12.6.4 The for-in Statement
Completion EvalForInStatement(AST* ast) {
  assert(ast->type() == AST::AST_STMT_FOR_IN);
  Error* e = Error::Ok();
  RuntimeContext::TopContext()->EnterIteration();
  ForIn* for_in_stmt = static_cast<ForIn*>(ast);
  JSObject* obj;
  JSValue* expr_ref;
  JSValue* expr_val;
  Completion stmt;
  bool has_label;
  JSValue* V = nullptr;
  if (for_in_stmt->expr0()->type() == AST::AST_STMT_VAR_DECL) {
    VarDecl* decl = static_cast<VarDecl*>(for_in_stmt->expr0());
    std::u16string var_name = EvalVarDecl(e, decl);
    if (!e->IsOk()) goto error;
    expr_ref = EvalExpression(e, for_in_stmt->expr1());
    if (!e->IsOk()) goto error;
    expr_val = GetValue(e, expr_ref);
    if (!e->IsOk()) goto error;
    if (expr_val->IsUndefined() || expr_val->IsNull()) {
      RuntimeContext::TopContext()->ExitIteration();
      return Completion(Completion::NORMAL, nullptr, u"");
    }
    obj = ToObject(e, expr_val);
    if (!e->IsOk()) goto error;

    for (auto pair : obj->AllEnumerableProperties()) {
      String* P = new String(pair.first);
      Reference* var_ref = IdentifierResolution(var_name);
      PutValue(e, var_ref, P);
      if (!e->IsOk()) goto error;

      stmt = EvalStatement(for_in_stmt->statement());
      if (stmt.value != nullptr)
        V = stmt.value;
      has_label = stmt.target == ast->label() || stmt.target == u"";
      if (stmt.type != Completion::CONTINUE || !has_label) {
        if (stmt.type == Completion::BREAK && has_label) {
          RuntimeContext::TopContext()->ExitIteration();
          return Completion(Completion::NORMAL, V, u"");
        }
        if (stmt.IsAbruptCompletion()) {
          RuntimeContext::TopContext()->ExitIteration();
          return stmt;
        }
      }
    }
  } else {
    expr_ref = EvalExpression(e, for_in_stmt->expr1());
    if (!e->IsOk()) goto error;
    expr_ref = EvalExpression(e, for_in_stmt->expr1());
    if (!e->IsOk()) goto error;
    expr_val = GetValue(e, expr_ref);
    if (!e->IsOk()) goto error;
    if (expr_val->IsUndefined() || expr_val->IsNull()) {
      RuntimeContext::TopContext()->ExitIteration();
      return Completion(Completion::NORMAL, nullptr, u"");
    }
    obj = ToObject(e, expr_val);
    for (auto pair : obj->AllEnumerableProperties()) {
      String* P = new String(pair.first);
      JSValue* lhs_ref = EvalExpression(e, for_in_stmt->expr0());
      if (!e->IsOk()) goto error;
      PutValue(e, lhs_ref, P);
      if (!e->IsOk()) goto error;

      stmt = EvalStatement(for_in_stmt->statement());
      if (stmt.value != nullptr)
        V = stmt.value;
      has_label = stmt.target == ast->label() || stmt.target == u"";
      if (stmt.type != Completion::CONTINUE || !has_label) {
        if (stmt.type == Completion::BREAK && has_label) {
          RuntimeContext::TopContext()->ExitIteration();
          return Completion(Completion::NORMAL, V, u"");
        }
        if (stmt.IsAbruptCompletion()) {
          RuntimeContext::TopContext()->ExitIteration();
          return stmt;
        }
      }
    }
  }
  RuntimeContext::TopContext()->ExitIteration();
  return Completion(Completion::NORMAL, V, u"");
error:
  RuntimeContext::TopContext()->ExitIteration();
  return Completion(Completion::THROW, new ErrorObject(e), u"");
}

Completion EvalContinueStatement(AST* ast) {
  assert(ast->type() == AST::AST_STMT_CONTINUE);
  Error* e = Error::Ok();
  if (!RuntimeContext::TopContext()->InIteration()) {
    *e = *Error::SyntaxError();
    return Completion(Completion::THROW, new ErrorObject(e), u"");
  }
  ContinueOrBreak* stmt = static_cast<ContinueOrBreak*>(ast);
  return Completion(Completion::CONTINUE, nullptr, stmt->ident());
}

Completion EvalBreakStatement(AST* ast) {
  assert(ast->type() == AST::AST_STMT_BREAK);
  Error* e = Error::Ok();
  if (!RuntimeContext::TopContext()->InIteration()) {
    *e = *Error::SyntaxError();
    return Completion(Completion::THROW, new ErrorObject(e), u"");
  }
  ContinueOrBreak* stmt = static_cast<ContinueOrBreak*>(ast);
  return Completion(Completion::BREAK, nullptr, stmt->ident());
}

Completion EvalReturnStatement(AST* ast) {
  assert(ast->type() == AST::AST_STMT_RETURN);
  Error* e = Error::Ok();
  Return* return_stmt = static_cast<Return*>(ast);
  if (return_stmt->expr() == nullptr) {
    return Completion(Completion::RETURN, Undefined::Instance(), u"");
  }
  auto exp_ref = EvalExpression(e, return_stmt->expr());
  return Completion(Completion::RETURN, GetValue(e, exp_ref), u"");
}

Completion EvalLabelledStatement(AST* ast) {
  assert(ast->type() == AST::AST_STMT_LABEL);
  LabelledStmt* label_stmt = static_cast<LabelledStmt*>(ast);
  label_stmt->statement()->SetLabel(label_stmt->label());
  Completion R = EvalStatement(label_stmt->statement());
  if (R.type == Completion::BREAK && R.target == label_stmt->label()) {
    return Completion(Completion::NORMAL, R.value, u"");
  }
  return R;
}

JSValue* EvalCaseClause(Error* e, Switch::CaseClause C) {
  JSValue* exp_ref = EvalExpression(e, C.expr);
  if (!e->IsOk())
    return nullptr;
  return GetValue(e, exp_ref);
}

Completion EvalCaseBlock(Switch* switch_stmt, JSValue* input) {
  Error* e = Error::Ok();
  JSValue* V = nullptr;
  bool found = false;
  for (auto C : switch_stmt->before_default_case_clauses()) {
    if (!found) {  // 5.a
      JSValue* clause_selector = EvalCaseClause(e, C);
      bool b = StrictEqual(e, input, clause_selector);
      if (!e->IsOk())
        return Completion(Completion::THROW, new ErrorObject(e), u"");
      if (b)
        found = true;
    }
    if (found) {  // 5.b
      Completion R = EvalStatementList(C.stmts);
      if (R.value != nullptr)
        V = R.value;
      if (R.IsAbruptCompletion())
        return Completion(R.type, V, R.target);
    }
  }
  bool found_in_b = false;
  size_t i;
  for (i = 0; !found_in_b && i < switch_stmt->after_default_case_clauses().size(); i++) {
    auto C = switch_stmt->after_default_case_clauses()[i];
    JSValue* clause_selector = EvalCaseClause(e, C);
    bool b = StrictEqual(e, input, clause_selector);
    if (!e->IsOk())
      return Completion(Completion::THROW, new ErrorObject(e), u"");
    if (b) {
      found_in_b = true;
      Completion R = EvalStatementList(C.stmts);
      if (R.value != nullptr)
        V = R.value;
      if (R.IsAbruptCompletion())
        return Completion(R.type, V, R.target);
    }
  }
  if (!found_in_b && switch_stmt->has_default_clause()) {  // 8
    Completion R = EvalStatementList(switch_stmt->default_clause().stmts);
    if (R.value != nullptr)
      V = R.value;
    if (R.IsAbruptCompletion())
      return Completion(R.type, V, R.target);
  }
  for (i = 0; i < switch_stmt->after_default_case_clauses().size(); i++) {
    auto C = switch_stmt->after_default_case_clauses()[i];
    JSValue* clause_selector = EvalCaseClause(e, C);
    Completion R = EvalStatementList(C.stmts);
    if (R.value != nullptr)
      V = R.value;
    if (R.IsAbruptCompletion())
      return Completion(R.type, V, R.target);
  }
  return Completion(Completion::NORMAL, V, u"");
}

// 12.11 The switch Statement
Completion EvalSwitchStatement(AST* ast) {
  assert(ast->type() == AST::AST_STMT_SWITCH);
  Error* e = Error::Ok();
  Switch* switch_stmt = static_cast<Switch*>(ast);
  JSValue* expr_ref = EvalExpression(e, switch_stmt->expr());
  if (!e->IsOk())
    return Completion(Completion::THROW, new ErrorObject(e), u"");
  Completion R = EvalCaseBlock(switch_stmt, expr_ref);
  if (R.IsThrow())
    return R;
  bool has_label = ast->label() == R.target;
  if (R.type == Completion::BREAK && has_label)
    return Completion(Completion::NORMAL, R.value, u"");
  return R;
}

// 12.13 The throw Statement
Completion EvalThrowStatement(AST* ast) {
  assert(ast->type() == AST::AST_STMT_THROW);
  Error* e = Error::Ok();
  Throw* throw_stmt = static_cast<Throw*>(ast);
  JSValue* exp_ref = EvalExpression(e, throw_stmt->expr());
  if (!e->Ok())
    return Completion(Completion::THROW, new ErrorObject(e), u"");
  JSValue* val = GetValue(e, exp_ref);
  if (!e->Ok())
    return Completion(Completion::THROW, new ErrorObject(e), u"");
  return Completion(Completion::THROW, val, u"");
}

Completion EvalCatch(Try* try_stmt, Completion C) {
  // NOTE(zhuzilin) Don't gc these two env, during this function.
  Error* e = Error::Ok();
  LexicalEnvironment* old_env = RuntimeContext::TopLexicalEnv();
  LexicalEnvironment* catch_env = LexicalEnvironment::NewDeclarativeEnvironment(old_env);
  catch_env->env_rec()->CreateMutableBinding(e, try_stmt->catch_ident(), false);  // 4
  if (!e->IsOk()) {
    return Completion(Completion::THROW, new ErrorObject(e), u"");
  }
  // NOTE(zhuzilin) The spec say to send C instead of C.value.
  // However, I think it should be send C.value...
  catch_env->env_rec()->SetMutableBinding(e, try_stmt->catch_ident(), C.value, false);  // 5
  if (!e->IsOk()) {
    return Completion(Completion::THROW, new ErrorObject(e), u"");
  }
  RuntimeContext::TopContext()->SetLexicalEnv(catch_env);
  Completion B = EvalBlockStatement(try_stmt->catch_block());
  RuntimeContext::TopContext()->SetLexicalEnv(old_env);
  return B;
}

Completion EvalTryStatement(AST* ast) {
  assert(ast->type() == AST::AST_STMT_TRY);
  Error* e = Error::Ok();
  Try* try_stmt = static_cast<Try*>(ast);
  Completion B = EvalBlockStatement(try_stmt->try_block());
  if (try_stmt->finally_block() == nullptr) {  // try Block Catch
    if (B.type != Completion::THROW)
      return B;
    return EvalCatch(try_stmt, B);
  } else if (try_stmt->catch_block() == nullptr) {  // try Block Finally
    Completion F = EvalBlockStatement(try_stmt->finally_block());
    if (F.type == Completion::NORMAL)
      return B;
    return F;
  } else {  // try Block Catch Finally
    Completion C = B;
    if (B.type == Completion::THROW) {
      C = EvalCatch(try_stmt, B);
    }
    Completion F = EvalBlockStatement(try_stmt->finally_block());
    if (F.type == Completion::NORMAL)
      return C;
    return F;
  }
}

Completion EvalExpressionStatement(AST* ast) {
  Error* e = Error::Ok();
  JSValue* val = EvalExpression(e, ast);
  if (!e->IsOk())
    return Completion(Completion::THROW, new ErrorObject(e), u"");
  return Completion(Completion::NORMAL, val, u"");
}

JSValue* EvalExpression(Error* e, AST* ast) {
  assert(ast->type() <= AST::AST_EXPR || ast->type() == AST::AST_FUNC);
  JSValue* val;
  switch (ast->type()) {
    case AST::AST_EXPR_THIS:
    case AST::AST_EXPR_IDENT:
    case AST::AST_EXPR_NULL:
    case AST::AST_EXPR_BOOL:
    case AST::AST_EXPR_NUMBER:
    case AST::AST_EXPR_STRING:
    case AST::AST_EXPR_OBJ:
    case AST::AST_EXPR_ARRAY:
    case AST::AST_EXPR_PAREN:
      val = EvalPrimaryExpression(e, ast);
      break;
    case AST::AST_EXPR_UNARY:
      val = EvalUnaryOperator(e, ast);
      break;
    case AST::AST_EXPR_BINARY:
      val = EvalBinaryExpression(e, ast);
      break;
    case AST::AST_EXPR_TRIPLE:
      val = EvalTripleConditionExpression(e, ast);
      break;
    case AST::AST_EXPR_LHS:
      val = EvalLeftHandSideExpression(e, ast);
      break;
    case AST::AST_EXPR:
      val = EvalExpressionList(e, ast);
    case AST::AST_FUNC:
      val = EvalFunction(e, ast);
      break;
    default:
      assert(false);
  }
  if (!e->IsOk()) return nullptr;
  return val;
}

JSValue* EvalPrimaryExpression(Error* e, AST* ast) {
  JSValue* val;
  switch (ast->type()) {
    case AST::AST_EXPR_THIS:
      val = RuntimeContext::TopContext()->this_binding();
      break;
    case AST::AST_EXPR_IDENT:
      val = EvalIdentifier(ast);
      break;
    case AST::AST_EXPR_NULL:
      val = Null::Instance();
      break;
    case AST::AST_EXPR_BOOL:
      val = ast->source() == u"true" ? Bool::True() : Bool::False();
      break;
    case AST::AST_EXPR_NUMBER:
      val = EvalNumber(ast);
      break;
    case AST::AST_EXPR_STRING:
      val = EvalString(ast);
      break;
    case AST::AST_EXPR_OBJ:
      val = EvalObject(e, ast);
      break;
    case AST::AST_EXPR_ARRAY:
      val = EvalArray(e, ast);
      break;
    case AST::AST_EXPR_PAREN:
      val = EvalExpression(e, static_cast<Paren*>(ast)->expr());
      break;
    default:
      std::cout << "Not primary expression, type " << ast->type() << std::endl;
      assert(false);
  }
  return val;
}

Reference* IdentifierResolution(std::u16string name) {
  // 10.3.1 Identifier Resolution
  LexicalEnvironment* env = RuntimeContext::TopLexicalEnv();
  bool strict = RuntimeContext::TopContext()->strict();
  return env->GetIdentifierReference(name, strict);
}

Reference* EvalIdentifier(AST* ast) {
  assert(ast->type() == AST::AST_EXPR_IDENT);
  return IdentifierResolution(ast->source());
}

Number* EvalNumber(std::u16string source) {
  double val = 0;
  double frac = 1;
  size_t pos = 0;
  bool dot = false;
  while (pos < source.size()) {
    char16_t c = source[pos];
    switch (c) {
      case u'.':
        dot = true;
        break;
      case u'e':
      case u'E': {
        double exp = 0;
        bool sign = true;
        pos++;  // skip e/E
        if (c == u'-') {
          sign = false;
          pos++;  // skip -
        } else if (c == u'+') {
          sign = true;
          pos++; // skip +;
        }
        while (pos < source.size()) {
          exp *= 10;
          exp += character::Digit(c);
        }
        if (!sign)
          exp = -exp;
        return new Number(val * pow(10.0, exp));
      }
      case u'x':
      case u'X': {
        assert(val == 0);
        pos++;
        while (pos < source.size()) {
          c = source[pos];
          val *= 16;
          val += character::Digit(c);
          pos++;
        }
        return new Number(val);
      }
      default:
        if (dot) {
          frac /= 10;
          val += character::Digit(c) * frac;
        } else {
          val *= 10;
          val += character::Digit(c);
        }
    }
    pos++;
  }
  return new Number(val);
}

Number* EvalNumber(AST* ast) {
  assert(ast->type() == AST::AST_EXPR_NUMBER);
  auto source = ast->source();
  return EvalNumber(source);
}

String* EvalString(std::u16string source) {
  source = source.substr(1, source.size() - 2);
  size_t pos = 0;
  std::vector<std::u16string> vals;
  while (pos < source.size()) {
    char16_t c = source[pos];
    switch (c) {
      case u'\\': {
        pos++;
        c = source[pos];
        switch (c) {
          case u'b':
            pos++;
            vals.emplace_back(u"\b");
            break;
          case u't':
            pos++;
            vals.emplace_back(u"\t");
            break;
          case u'n':
            pos++;
            vals.emplace_back(u"\n");
            break;
          case u'v':
            pos++;
            vals.emplace_back(u"\v");
            break;
          case u'f':
            pos++;
            vals.emplace_back(u"\f");
            break;
          case u'r':
            pos++;
            vals.emplace_back(u"\r");
            break;
          case u'x': {
            pos++;  // skip 'x'
            char16_t hex = 0;
            for (size_t i = 0; i < 2; i++) {
              hex *= 16;
              hex += character::Digit(source[pos]);
              pos++;
            }
            vals.emplace_back(std::u16string(1, hex));
            break;
          }
          case u'u': {
            pos++;  // skip 'u'
            char16_t hex = 0;
            for (size_t i = 0; i < 4; i++) {
              hex *= 16;
              hex += character::Digit(source[pos]);
              pos++;
            }
            vals.emplace_back(std::u16string(1, hex));
            break;
          }
          default:
            c = source[pos];
            if (character::IsLineTerminator(c)) {
              pos++;
              continue;
            }
            pos++;
            vals.emplace_back(std::u16string(1, c));
        }
        break;
      }
      default: {
        size_t start = pos;
        while (true) {
          if (pos == source.size() || source[pos] == u'\\')
            break;
          pos++;
        }
        size_t end = pos;
        auto substr = source.substr(start, end - start);
        vals.emplace_back(std::u16string(substr.data(), substr.size()));
      }
    }
  }
  if (vals.size() == 0) {
    return String::Empty();
  } else if (vals.size() == 1) {
    return new String(vals[0]);
  }
  return new String(StrCat(vals));
}

String* EvalString(AST* ast) {
  assert(ast->type() == AST::AST_EXPR_STRING);
  auto source = ast->source();
  return EvalString(source);
}

std::u16string EvalPropertyName(Error* e, Token token) {
  switch (token.type()) {
    case Token::TK_IDENT:
    case Token::TK_KEYWORD:
    case Token::TK_FUTURE:
      return token.source();
    case Token::TK_NUMBER:
      return ToString(e, EvalNumber(token.source()));
    case Token::TK_STRING:
      return ToString(e, EvalString(token.source()));
    default:
      assert(false);
  }
}

Object* EvalObject(Error* e, AST* ast) {
  assert(ast->type() == AST::AST_EXPR_OBJ);
  ObjectLiteral* obj_ast = static_cast<ObjectLiteral*>(ast);
  bool strict = RuntimeContext::TopContext()->strict();
  Object* obj = new Object();
  // PropertyName : AssignmentExpression
  for (auto property : obj_ast->properties()) {
    std::u16string prop_name = EvalPropertyName(e, property.key);
    PropertyDescriptor* desc = new PropertyDescriptor();
    switch (property.type) {
      case ObjectLiteral::Property::NORMAL: {
        JSValue* expr_value = EvalAssignmentExpression(e, property.value);
        JSValue* prop_value = GetValue(e, expr_value);
        desc->SetDataDescriptor(prop_value, true, true, true);
        break;
      }
      default: {
        assert(property.value->type() == AST::AST_FUNC);
        Function* func_ast = static_cast<Function*>(property.value);
        bool strict_func = static_cast<ProgramOrFunctionBody*>(func_ast->body())->strict();
        if (strict || strict_func) {
          for (auto name : func_ast->params()) {
            if (name == u"eval" || name == u"arguments") {
              *e = *Error::SyntaxError();
              return nullptr;
            }
          }
        }
        FunctionObject* closure = new FunctionObject(
          func_ast->params(), func_ast->body(),
          RuntimeContext::TopLexicalEnv()
        );
        if (property.type == ObjectLiteral::Property::GET) {
          desc->SetGet(closure);
        } else {
          desc->SetSet(closure);
        }
        desc->SetEnumerable(true);
        desc->SetConfigurable(true);
        break;
      }
    }
    auto previous = obj->GetOwnProperty(prop_name);  // 3
    if (!previous->IsUndefined()) {  // 4
      PropertyDescriptor* previous_desc = static_cast<PropertyDescriptor*>(previous);
      if (strict &&
          previous_desc->IsDataDescriptor() && desc->IsDataDescriptor()) {  // 4.a
        *e = *Error::SyntaxError();
        return nullptr;
      }
      if (previous_desc->IsDataDescriptor() && desc->IsAccessorDescriptor() ||  // 4.b
          previous_desc->IsAccessorDescriptor() && desc->IsDataDescriptor()) {  // 4.c
        *e = *Error::SyntaxError();
        return nullptr;
      }
      if (previous_desc->IsAccessorDescriptor() && desc->IsAccessorDescriptor() &&  // 4.d
          (previous_desc->HasGet() && desc->HasGet() || previous_desc->HasSet() && desc->HasSet())) {
        *e = *Error::SyntaxError();
        return nullptr;
      }
    }
    obj->DefineOwnProperty(e, prop_name, desc, false);
  }
  return obj;
}

ArrayObject* EvalArray(Error* e, AST* ast) {
  assert(ast->type() == AST::AST_EXPR_ARRAY);
  ArrayLiteral* array_ast = static_cast<ArrayLiteral*>(ast);
  
  ArrayObject* arr = new ArrayObject(array_ast->length());
  for (auto pair : array_ast->elements()) {
    JSValue* init_result = EvalAssignmentExpression(e, pair.second);
    if (!e->IsOk()) return nullptr;
    arr->AddValueProperty(NumberToString(pair.first), init_result, true, true, true);
  }
  return arr;
}

JSValue* EvalAssignmentExpression(Error* e, AST* ast) {
  return EvalExpression(e, ast);
}

JSValue* EvalUnaryOperator(Error* e, AST* ast) {
  assert(ast->type() == AST::AST_EXPR_UNARY);
  Unary* u = static_cast<Unary*>(ast);

  JSValue* expr = EvalExpression(e, u->node());
  if (!e->IsOk()) return nullptr;
  std::u16string op = u->op().source();

  if (op == u"++" || op == u"--") {  // a++, ++a, a--, --a
    if (expr->IsReference()) {
      Reference* ref = static_cast<Reference*>(expr);
      if (ref->IsStrictReference() && ref->GetBase()->IsEnvironmentRecord() &&
          (ref->GetReferencedName() == u"eval" || ref->GetReferencedName() == u"arguments")) {
        *e = *Error::SyntaxError();
        return nullptr;
      }
    }
    JSValue* old_val = GetValue(e, expr);
    if (!e->IsOk()) return nullptr;
    double num = ToNumber(e, old_val);
    if (!e->IsOk()) return nullptr;
    JSValue* new_value;
    if (op == u"++") {
      new_value = new Number(num + 1);
    } else {
      new_value = new Number(num - 1);
    }
    PutValue(e, expr, new_value);
    if (!e->IsOk()) return nullptr;
    if (u->prefix()) {
      return new_value;
    } else {
      return old_val;
    }
  } else if (op == u"delete") {  // 11.4.1 The delete Operator
    if (!expr->IsReference())  // 2
      return Bool::True();
    Reference* ref = static_cast<Reference*>(expr);
    if (ref->IsUnresolvableReference()) {  // 3
      if (ref->IsStrictReference()) {
        *e = *Error::SyntaxError();
        return Bool::False();
      }
      return Bool::True();
    }
    if (ref->IsPropertyReference()) {  // 4
      JSObject* obj = ToObject(e, ref->GetBase());
      if (!e->IsOk()) return nullptr;
      return Bool::Wrap(obj->Delete(e, ref->GetReferencedName(), ref->IsStrictReference()));
    } else {
      if (ref->IsStrictReference()) {
        *e = *Error::SyntaxError();
        return Bool::False();
      }
      EnvironmentRecord* bindings = static_cast<EnvironmentRecord*>(ref->GetBase());
      return Bool::Wrap(bindings->DeleteBinding(e, ref->GetReferencedName()));
    }
  } else if (op == u"typeof") {
    if (expr->IsReference()) {
      Reference* ref = static_cast<Reference*>(expr);
      if (ref->IsUnresolvableReference())
        return String::Undefined();
    }
    JSValue* val = GetValue(e, expr);
    if (!e->IsOk()) return nullptr;
    switch (val->type()) {
      case JSValue::JS_UNDEFINED:
        return String::Undefined();
      case JSValue::JS_NULL:
        return new String(u"object");
      case JSValue::JS_NUMBER:
        return new String(u"number");
      case JSValue::JS_STRING:
        return new String(u"string");
      default:
        if (val->IsCallable())
          return new String(u"function");
        return new String(u"object");
    }
  } else {  // +, -, ~, !, void
    JSValue* val = GetValue(e, expr);
    if (!e->IsOk()) return nullptr;

    if (op == u"+") {
      double num = ToNumber(e, val);
      if (!e->IsOk()) return nullptr;
      return new Number(num);
    } else if (op == u"-") {
      double num = ToNumber(e, val);
      if (!e->IsOk()) return nullptr;
      if (isnan(num))
        return Number::NaN();
      return new Number(-num);
    } else if (op == u"~") {
      int32_t num = ToInt32(e, val);
      if (!e->IsOk()) return nullptr;
      return new Number(~num);
    } else if (op == u"!") {
      bool b = ToBoolean(val);
      return Bool::Wrap(!b);
    } else if (op == u"void") {
      return Undefined::Instance();
    }
  }
  assert(false);
}

JSValue* EvalBinaryExpression(Error* e, AST* ast) {
  assert(ast->type() == AST::AST_EXPR_BINARY);
  Binary* b = static_cast<Binary*>(ast);
  return EvalBinaryExpression(e, b->op(), b->lhs(), b->rhs());
}

JSValue* EvalBinaryExpression(Error* e, std::u16string op, AST* lhs, AST* rhs) {
  // && and || are different, as there are not &&= and ||=
  if (op == u"&&" || op == u"||") {
    return EvalLogicalOperator(e, op, lhs, rhs);
  }
  if (op == u"=" || op == u"*=" || op == u"/=" || op == u"%=" ||
      op == u"+=" || op == u"-=" || op == u"<<=" || op == u">>=" ||
      op == u">>>=" || op == u"&=" || op == u"^=" || op == u"|=") {
    JSValue* lref = EvalLeftHandSideExpression(e, lhs);
    if (!e->IsOk()) return nullptr;
    // TODO(zhuzilin) The compound assignment should do lval = GetValue(lref)
    // here. Check if changing the order will have any influence.
    JSValue* rref = EvalExpression(e, rhs);
    if (!e->IsOk()) return nullptr;
    JSValue* rval = GetValue(e, rref);
    if (!e->IsOk()) return nullptr;
    if (op == u"=") {
      return EvalSimpleAssignment(e, lref, rval);
    } else {
      return EvalCompoundAssignment(e, op, lref, rval);
    }
  }

  JSValue* lref = EvalExpression(e, lhs);
  if (!e->IsOk()) return nullptr;
  JSValue* lval = GetValue(e, lref);
  if (!e->IsOk()) return nullptr;
  JSValue* rref = EvalExpression(e, rhs);
  if (!e->IsOk()) return nullptr;
  JSValue* rval = GetValue(e, rref);
  if (!e->IsOk()) return nullptr;
  return EvalBinaryExpression(e, op, lval, rval);
}

JSValue* EvalBinaryExpression(Error* e, std::u16string op, JSValue* lval, JSValue* rval) {
  if (op == u"*" || op == u"/" || op == u"%" || op == u"-") {
    return EvalArithmeticOperator(e, op, lval, rval);
  } else if (op == u"+") {
    return EvalAddOperator(e, lval, rval);
  } else if (op == u"<<" || op == u">>" || op == u">>>") {
    return EvalBitwiseShiftOperator(e, op, lval, rval);
  } else if (op == u"<" || op == u">" || op == u"<=" || op == u">=" ||
             op == u"instanceof" || op == u"in") {
    return EvalRelationalOperator(e, op, lval, rval);
  } else if (op == u"==" || op == u"!=" || op == u"===" || op == u"!==") {
    return EvalEqualityOperator(e, op, lval, rval);
  } else if (op == u"&" || op == u"^" || op == u"|") {
    return EvalBitwiseOperator(e, op, lval, rval);
  }
  assert(false);
}

// 11.5 Multiplicative Operators
JSValue* EvalArithmeticOperator(Error* e, std::u16string op, JSValue* lval, JSValue* rval) {
  double lnum = ToNumber(e, lval);
  if (!e->IsOk()) return nullptr;
  double rnum = ToNumber(e, rval);
  if (!e->IsOk()) return nullptr;
  switch (op[0]) {
    case u'*':
      return new Number(lnum * rnum);
    case u'/':
      return new Number(lnum / rnum);
    case u'%':
      return new Number(fmod(lnum, rnum));
    case u'-':
      return new Number(lnum - rnum);
    default:
      assert(false);
  }
}

// 11.6 Additive Operators
JSValue* EvalAddOperator(Error* e, JSValue* lval, JSValue* rval) {
  JSValue* lprim = ToPrimitive(e, lval, u"");
  if (!e->IsOk()) return nullptr;
  JSValue* rprim = ToPrimitive(e, rval, u"");
  if (!e->IsOk()) return nullptr;

  if (lprim->IsString() || rprim->IsString()) {
    std::u16string lstr = ToString(e, lprim);
    if (!e->IsOk()) return nullptr;
    std::u16string rstr = ToString(e, rprim);
    if (!e->IsOk()) return nullptr;
    return new String(lstr + rstr);
  }

  double lnum = ToNumber(e, lprim);
  if (!e->IsOk()) return nullptr;
  double rnum = ToNumber(e, rprim);
  if (!e->IsOk()) return nullptr;
  return new Number(lnum + rnum);
}

// 11.7 Bitwise Shift Operators
JSValue* EvalBitwiseShiftOperator(Error* e, std::u16string op, JSValue* lval, JSValue* rval) {
  int32_t lnum = ToInt32(e, lval);
  if (!e->IsOk()) return nullptr;
  uint32_t rnum = ToUint32(e, rval);
  if (!e->IsOk()) return nullptr;
  uint32_t shift_count = rnum & 0x1F;
  if (op == u"<<") {
    return new Number(lnum << shift_count);
  } else if (op == u">>") {
    return new Number(lnum >> shift_count);
  } else if (op == u">>>") {
    uint32_t lnum = ToUint32(e, lval);
    return new Number(lnum >> rnum);
  }
  assert(false);
}

// 11.8 Relational Operators
JSValue* EvalRelationalOperator(Error* e, std::u16string op, JSValue* lval, JSValue* rval) {
  if (op == u"<") {
    JSValue* r = LessThan(e, lval, rval);
    if (!e->IsOk()) return nullptr;
    return r->IsUndefined() ? Bool::False() : r;
  } else if (op == u">") {
    JSValue* r = LessThan(e, rval, lval);
    if (!e->IsOk()) return nullptr;
    return r->IsUndefined() ? Bool::False() : r;
  } else if (op == u"<=") {
    JSValue* r = LessThan(e, rval, lval);
    if (!e->IsOk()) return nullptr;
    if (r->IsUndefined())
      return Bool::True();
    return Bool::Wrap(!static_cast<Bool*>(r)->data());
  } else if (op == u">=") {
    JSValue* r = LessThan(e, lval, rval);
    if (!e->IsOk()) return nullptr;
    if (r->IsUndefined())
      return Bool::True();
    return Bool::Wrap(!static_cast<Bool*>(r)->data());
  } else if (op == u"instanceof") {
    if (!rval->IsObject()) {
      *e = *Error::TypeError(u"instanceof called on non-object");
      return nullptr;
    }
    JSObject* obj = static_cast<JSObject*>(rval);
    if (!obj->IsFunction()) {  // only FunctionObject has [[HasInstance]]
      *e = *Error::TypeError(u"only FunctionObject has [[HasInstance]]");
      return nullptr;
    }
    return Bool::Wrap(obj->HasInstance(e, lval));
  } else if (op == u"in") {
    if (!rval->IsObject()) {
      *e = *Error::TypeError(u"in called on non-object");
      return nullptr;
    }
    JSObject* obj = static_cast<JSObject*>(rval);
    return Bool::Wrap(obj->HasProperty(ToString(e, lval)));
  }
  assert(false);
}

// 11.9 Equality Operators
JSValue* EvalEqualityOperator(Error* e, std::u16string op, JSValue* lval, JSValue* rval) {
  if (op == u"==") {
    return Bool::Wrap(Equal(e, lval, rval));
  } else if (op == u"!=") {
    return Bool::Wrap(!Equal(e, lval, rval));
  } else if (op == u"===") {
    return Bool::Wrap(StrictEqual(e, lval, rval));
  } else if (op == u"!==") {
    return Bool::Wrap(!StrictEqual(e, lval, rval));
  }
  assert(false);
}

// 11.10 Binary Bitwise Operators
JSValue* EvalBitwiseOperator(Error* e, std::u16string op, JSValue* lval, JSValue* rval) {
  int32_t lnum = ToInt32(e, lval);
  if (!e->IsOk()) return nullptr;
  int32_t rnum = ToInt32(e, rval);
  if (!e->IsOk()) return nullptr;
  switch (op[0]) {
    case u'&':
      return new Number(lnum & rnum);
    case u'^':
      return new Number(lnum ^ rnum);
    case u'|':
      return new Number(lnum | rnum);
    default:
      assert(false);
  }
}

// 11.11 Binary Logical Operators
JSValue* EvalLogicalOperator(Error* e, std::u16string op, AST* lhs, AST* rhs) {
  JSValue* lref = EvalExpression(e, lhs);
  if (!e->IsOk()) return nullptr;
  JSValue* lval = GetValue(e, lref);
  if (!e->IsOk()) return nullptr;
  if (op == u"&&" && !ToBoolean(lval) || op == u"||" && ToBoolean(lval))
    return lval;
  JSValue* rref = EvalExpression(e, rhs);
  if (!e->IsOk()) return nullptr;
  JSValue* rval = GetValue(e, rref);
  if (!e->IsOk()) return nullptr;
  return rval;
}

// 11.13.1 Simple Assignment ( = )
JSValue* EvalSimpleAssignment(Error* e, JSValue* lref, JSValue* rval) {
  if (lref->type() == JSValue::JS_REF) {
    Reference* ref = static_cast<Reference*>(lref);
    // NOTE in 11.13.1.
    // TODO(zhuzilin) not sure how to implement the type error part of the note.
    if (ref->IsStrictReference() && ref->IsUnresolvableReference()) {
      *e = *Error::ReferenceError(ref->GetReferencedName() + u" is not defined");
      return nullptr;
    }
    if (ref->IsStrictReference() && ref->GetBase()->type() == JSValue::JS_ENV_REC &&
        (ref->GetReferencedName() == u"eval" || ref->GetReferencedName() == u"arguments")) {
      *e = *Error::SyntaxError();
      return nullptr;
    }
  }
  PutValue(e, lref, rval);
  if (!e->IsOk())
    return nullptr;
  return rval;
}

// 11.13.2 Compound Assignment ( op= )
JSValue* EvalCompoundAssignment(Error* e, std::u16string op, JSValue* lref, JSValue* rval) {
  std::u16string calc_op = op.substr(0, op.size() - 1);
  JSValue* lval = GetValue(e, lref);
  if (!e->IsOk()) return nullptr;
  JSValue* r = EvalBinaryExpression(e, calc_op, lval, rval);
  if (!e->IsOk()) return nullptr;
  return EvalSimpleAssignment(e, lref, r);
}

// 11.12 Conditional Operator ( ? : )
JSValue* EvalTripleConditionExpression(Error* e, AST* ast) {
  assert(ast->type() == AST::AST_EXPR_TRIPLE);
  TripleCondition* t = static_cast<TripleCondition*>(ast);
  JSValue* lref = EvalExpression(e, t->cond());
  if (!e->IsOk()) return nullptr;
  JSValue* lval = GetValue(e, lref);
  if (!e->IsOk()) return nullptr;
  if (ToBoolean(lval)) {
    JSValue* true_ref = EvalAssignmentExpression(e, t->true_expr());
    if (!e->IsOk()) return nullptr;
    return GetValue(e, true_ref);
  } else {
    JSValue* false_ref = EvalAssignmentExpression(e, t->false_expr());
    if (!e->IsOk()) return nullptr;
    return GetValue(e, false_ref);
  }
}

JSValue* EvalLeftHandSideExpression(Error* e, AST* ast) {
  assert(ast->type() == AST::AST_EXPR_LHS);
  LHS* lhs = static_cast<LHS*>(ast);

  size_t new_count = lhs->new_count();
  size_t base_offset = lhs->order().size();
  if (base_offset > 0) {
    for (size_t i = 0; i < new_count; i++) {
      auto pair = lhs->order()[base_offset - 1];
      if (pair.second == LHS::PostfixType::CALL) {
        base_offset--;
        if (base_offset == 0)
          break;
      } else {
        break;
      }
    }
  }

  ValueGuard guard;
  JSValue* base = EvalExpression(e, lhs->base());
  if (!e->IsOk()) return nullptr;
  for (size_t i = 0; i < base_offset; i++) {
    if (base == nullptr)
      return base;
    auto pair = lhs->order()[i];
    switch (pair.second) {
      case LHS::PostfixType::CALL: {
        auto args = lhs->args_list()[pair.first];
        auto arg_list = EvalArgumentsList(e, args);
        if (!e->IsOk()) return nullptr;
        base = EvalCallExpression(e, base, arg_list);
        if (!e->IsOk()) return nullptr;
        break;
      }
      case LHS::PostfixType::INDEX: {
        auto index = lhs->index_list()[pair.first];
        base = EvalIndexExpression(e, base, index, guard);
        if (!e->IsOk()) return nullptr;
        break;
      }
      case LHS::PostfixType::PROP: {
        auto prop = lhs->prop_name_list()[pair.first];
        base = EvalIndexExpression(e, base, prop, guard);
        if (!e->IsOk()) return nullptr;
        break;
      }
      default:
        assert(false);
    }

  }
  // NewExpression
  for (size_t i = 0; i < new_count; i++) {
    base = GetValue(e, base);
    if (!e->IsOk()) return nullptr;
    if (!base->IsConstructor()) {
      *e = *Error::TypeError(u"base value is not a constructor");
      return nullptr;
    }
    JSObject* constructor = static_cast<JSObject*>(base);
    std::vector<JSValue*> arg_list;
    if (base_offset < lhs->order().size()) {
      auto pair = lhs->order()[base_offset];
      assert(pair.second == LHS::PostfixType::CALL);
      auto args = lhs->args_list()[pair.first];
      arg_list = EvalArgumentsList(e, args);
      base_offset++;
    }
    base = constructor->Construct(e, arg_list);
    if (!e->IsOk()) return nullptr;
  }
  return base;
}

std::vector<JSValue*> EvalArgumentsList(Error* e, Arguments* ast) {
  std::vector<JSValue*> arg_list;
  for (AST* ast : ast->args()) {
    JSValue* ref = EvalExpression(e, ast);
    if (!e->IsOk())
      return {};
    JSValue* arg = GetValue(e, ref);
    if (!e->IsOk())
      return {};
    arg_list.emplace_back(arg);
  }
  return arg_list;
}

// 11.2.3
JSValue* EvalCallExpression(Error* e, JSValue* ref, std::vector<JSValue*> arg_list) {
  JSValue* val = GetValue(e, ref);
  if (!e->IsOk())
    return nullptr;
  if (!val->IsObject()) {  // 4
    *e = *Error::TypeError(u"is not a function");
    return nullptr;
  }
  auto obj = static_cast<JSObject*>(val);
  if (!obj->IsCallable()) {  // 5
    *e = *Error::TypeError(u"is not a function");
    return nullptr;
  }
  JSValue* this_value;
  if (ref->IsReference()) {
    Reference* r = static_cast<Reference*>(ref);
    JSValue* base = r->GetBase();
    if (r->IsPropertyReference()) {
      this_value = base;
    } else {
      assert(base->IsEnvironmentRecord());
      auto env_rec = static_cast<EnvironmentRecord*>(base);
      this_value = env_rec->ImplicitThisValue();
    }
  } else {
    this_value = Undefined::Instance();
  }
  // indirect 
  if (ref->IsReference() && static_cast<Reference*>(ref)->GetReferencedName() == u"eval") {
    DirectEvalGuard guard;
    return obj->Call(e, this_value, arg_list);
  } else {
    return obj->Call(e, this_value, arg_list);
  }
}

// 11.2.1 Property Accessors
JSValue* EvalIndexExpression(Error* e, JSValue* base_ref, std::u16string identifier_name, ValueGuard& guard) {
  JSValue* base_value = GetValue(e, base_ref);
  if (!e->IsOk())
    return nullptr;
  guard.AddValue(base_value);
  base_value->CheckObjectCoercible(e);
  if (!e->IsOk())
    return nullptr;
  bool strict = RuntimeContext::TopContext()->strict();
  return new Reference(base_value, identifier_name, strict);
}

JSValue* EvalIndexExpression(Error* e, JSValue* base_ref, AST* expr, ValueGuard& guard) {
  JSValue* property_name_ref = EvalExpression(e, expr);
  if (!e->IsOk())
    return nullptr;
  JSValue* property_name_value = GetValue(e, property_name_ref);
  if (!e->IsOk())
    return nullptr;
  std::u16string property_name_str = ToString(e, property_name_value);
  if (!e->IsOk())
    return nullptr;
  return EvalIndexExpression(e, base_ref, property_name_str, guard);
}

JSValue* EvalExpressionList(Error* e, AST* ast) {
  assert(ast->type() == AST::AST_EXPR);
  Expression* exprs = static_cast<Expression*>(ast);
  assert(exprs->elements().size() > 0);
  JSValue* val;
  for (AST* expr : exprs->elements()) {
    JSValue* ref = EvalAssignmentExpression(e, expr);
    if (!e->IsOk()) return nullptr;
    val = GetValue(e, ref);
    if (!e->IsOk()) return nullptr;
  }
  return val;
}

}  // namespace es

#endif  // ES_EVALUATOR_H