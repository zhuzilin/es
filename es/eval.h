#ifndef ES_EVALUATOR_H
#define ES_EVALUATOR_H

#include <math.h>

#include <es/parser/character.h>
#include <es/parser/ast.h>
#include <es/types/completion.h>
#include <es/types/reference.h>
#include <es/types/builtin/function_object.h>
#include <es/types/builtin/error_object.h>
#include <es/types/compare.h>
#include <es/execution_context.h>
#include <es/helper.h>

namespace es {

Completion EvalProgram(Error* e, AST* ast);

Completion EvalStatement(Error* e, AST* ast);
Completion EvalBlockStatement(Error* e, AST* ast);
Completion EvalIfStatement(Error* e, AST* ast);
Completion EvalReturnStatement(Error* e, AST* ast);
Completion EvalVarStatement(Error* e, AST* ast);

Completion EvalExpressionStatement(Error* e, AST* ast);

JSValue* EvalExpression(Error* e, AST* ast);
JSValue* EvalPrimaryExpression(Error* e, AST* ast);
Reference* EvalIdentifier(AST* ast);
Number* EvalNumber(AST* ast);
String* EvalString(AST* ast);
Object* EvalObject(Error* e, AST* ast);
JSValue* EvalAssignmentExpression(Error* e, AST* ast);

JSValue* EvalUnaryOperator(Error* e, AST* ast);

JSValue* EvalBinaryExpression(Error* e, AST* ast);
JSValue* EvalSimpleAssignment(Error* e, AST* lhs, AST* rhs);
JSValue* EvalArithmeticOperator(Error* e, std::u16string op, AST* lhs, AST* rhs);
JSValue* EvalAddOperator(Error* e, AST* lhs, AST* rhs);
JSValue* EvalBitwiseShiftOperator(Error* e, std::u16string op, AST* lhs, AST* rhs);
JSValue* EvalRelationalOperator(Error* e, std::u16string op, AST* lhs, AST* rhs);
JSValue* EvalEqualityOperator(Error* e, std::u16string op, AST* lhs, AST* rhs);
JSValue* EvalBitwiseOperator(Error* e, std::u16string op, AST* lhs, AST* rhs);
JSValue* EvalLogicalOperator(Error* e, std::u16string op, AST* lhs, AST* rhs);

JSValue* EvalLeftHandSideExpression(Error* e, AST* ast);
std::vector<JSValue*> EvalArgumentsList(Error* e, Arguments* ast);
JSValue* EvalCallExpression(Error* e, JSValue* ref, std::vector<JSValue*> arg_list);
JSValue* EvalIndexExpression(Error* e, JSValue* base_ref, std::u16string identifier_name);
JSValue* EvalIndexExpression(Error* e, JSValue* base_ref, AST* expr);

Reference* IdentifierResolution(std::u16string name);

Completion EvalProgram(Error* e, AST* ast) {
  assert(ast->type() == AST::AST_PROGRAM || ast->type() == AST::AST_FUNC_BODY);
  auto prog = static_cast<ProgramOrFunctionBody*>(ast);
  auto statements = prog->statements();
  // 12.9 considered syntactically incorrect if it contains
  //      a return statement that is not within a FunctionBody.
  if (ast->type() != AST::AST_FUNC_BODY) {
    for (auto stmt : statements) {
      if (stmt->type() == AST::AST_STMT_RETURN) {
        *e = *Error::SyntaxError();
        return Completion(Completion::THROW, new ErrorObject(e), nullptr);
      }
    }
  }

  Completion head_result, tail_result;
  if (statements.size() == 0)
    return Completion();
  for (auto stmt : prog->statements()) {
    if (head_result.IsAbruptCompletion())
      break;
    tail_result = EvalStatement(e, stmt);
    if (!e->IsOk())
      return Completion(Completion::THROW, new ErrorObject(e), nullptr);
    head_result = Completion(
      tail_result.type,
      tail_result.value == nullptr? head_result.value : tail_result.value,
      tail_result.target
    );
  }
  return head_result;
}

Completion EvalStatement(Error* e, AST* ast) {
  switch(ast->type()) {
    case AST::AST_STMT_BLOCK:
      return EvalBlockStatement(e, ast);
    case AST::AST_STMT_IF:
      return EvalIfStatement(e, ast);
    case AST::AST_STMT_RETURN:
      return EvalReturnStatement(e, ast);
    case AST::AST_STMT_VAR:
      return EvalVarStatement(e, ast);
    case AST::AST_STMT_EMPTY:
      return Completion();
    default:
      return EvalExpressionStatement(e, ast);
  }
}

Completion EvalBlockStatement(Error* e, AST* ast) {
  assert(ast->type() == AST::AST_STMT_BLOCK);
  Block* block = static_cast<Block*>(ast);
  Completion sl;
  for (auto stmt : block->statements()) {
    Completion s = EvalStatement(e, stmt);
    if (!e->IsOk()) {
      // TODO(zhuzilin) error object
      return Completion(Completion::THROW, nullptr, nullptr);
    }
    sl = Completion(s.type, s.value == nullptr ? sl.value : s.value, s.target);
    if (sl.IsAbruptCompletion())
      return sl;
  }
  return sl;
}

Completion EvalIfStatement(Error* e, AST* ast) {
  assert(ast->type() == AST::AST_STMT_IF);
  If* if_stmt = static_cast<If*>(ast);
  JSValue* expr_ref = EvalExpression(e, if_stmt->cond());
  if (!e->IsOk())
    return Completion(Completion::THROW, nullptr, nullptr);
  JSValue* exp = GetValue(e, expr_ref);
  if (!e->IsOk())
    return Completion(Completion::THROW, nullptr, nullptr);

  if (ToBoolean(exp)) {
    return EvalStatement(e, if_stmt->if_block());
  } else if (if_stmt->else_block() != nullptr){
    return EvalStatement(e, if_stmt->else_block());
  }
  return Completion(Completion::NORMAL, nullptr, nullptr);
}

Completion EvalReturnStatement(Error* e, AST* ast) {
  log::PrintSource("enter EvalReturnStatement: ", ast->source());
  assert(ast->type() == AST::AST_STMT_RETURN);
  Return* return_stmt = static_cast<Return*>(ast);
  if (return_stmt->expr() == nullptr) {
    return Completion(Completion::RETURN, Undefined::Instance(), nullptr);
  }
  auto exp_ref = EvalExpression(e, return_stmt->expr());
  return Completion(Completion::RETURN, GetValue(e, exp_ref), nullptr);
}

Completion EvalVarStatement(Error* e, AST* ast) {
  log::PrintSource("enter EvalVarStatement: ", ast->source());
  assert(ast->type() == AST::AST_STMT_VAR);
  VarStmt* var_stmt = static_cast<VarStmt*>(ast);
  for (VarDecl* decl : var_stmt->decls()) {
    if (decl->init() == nullptr)
      continue;
    JSValue* lhs = IdentifierResolution(decl->ident());
    JSValue* rhs = EvalAssignmentExpression(e, decl->init());
    if (!e->IsOk()) goto error;
    JSValue* value = GetValue(e, rhs);
    if (!e->IsOk()) goto error;
    PutValue(e, lhs, value);
    if (!e->IsOk()) goto error;
  }
  return Completion(Completion::NORMAL, nullptr, nullptr);
error:
  return Completion(Completion::THROW, new ErrorObject(e), nullptr);
}

Completion EvalExpressionStatement(Error* e, AST* ast) {
  JSValue* val = EvalExpression(e, ast);
  return Completion(Completion::NORMAL, val, nullptr);
}

JSValue* EvalExpression(Error* e, AST* ast) {
  JSValue* val;
  switch (ast->type()) {
    case AST::AST_EXPR_THIS:
    case AST::AST_EXPR_IDENT:
    case AST::AST_EXPR_NULL:
    case AST::AST_EXPR_BOOL:
    case AST::AST_EXPR_NUMBER:
    case AST::AST_EXPR_STRING:
    case AST::AST_EXPR_OBJ:
    case AST::AST_EXPR_PAREN:
      val = EvalPrimaryExpression(e, ast);
      break;
    case AST::AST_EXPR_UNARY:
      val = EvalUnaryOperator(e, ast);
      break;
    case AST::AST_EXPR_BINARY:
      val = EvalBinaryExpression(e, ast);
      break;
    case AST::AST_EXPR_LHS:
      val = EvalLeftHandSideExpression(e, ast);
      break;
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
      val = ExecutionContextStack::Global()->Top().this_binding();
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
    case AST::AST_EXPR_PAREN:
      val = EvalExpression(e, static_cast<Paren*>(ast)->expr());
      break;
    default:
      std::cout << ast->type() << " " << AST::AST_ILLEGAL << std::endl;
      assert(false);
  }
  return val;
}

Reference* IdentifierResolution(std::u16string name) {
  log::PrintSource("enter IdentifierResolution ", name);
  // 10.3.1 Identifier Resolution
  LexicalEnvironment* env = ExecutionContextStack::Global()->TopLexicalEnv();
  bool strict = ExecutionContextStack::Global()->Top().strict();
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
    return new String(u"");
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
  bool strict = ExecutionContextStack::Global()->Top().strict();
  Object* obj = new Object();
  // PropertyName : AssignmentExpression
  for (auto property : obj_ast->properties()) {
    std::u16string prop_name = EvalPropertyName(e, property.key);
    log::PrintSource("EvalObject: adding property ", prop_name);
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
          ExecutionContextStack::Global()->TopLexicalEnv()
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
    JSValue* val = GetValue(e, expr);
    if (!e->IsOk()) return nullptr;
    double num = ToNumber(e, val);
    if (!e->IsOk()) return nullptr;
    if (op == u"++") {
      PutValue(e, expr, new Number(num++));
    } else {
      PutValue(e, expr, new Number(num--));
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
        return new String(u"Object");
      case JSValue::JS_NUMBER:
        return new String(u"Number");
      case JSValue::JS_STRING:
        return new String(u"String");
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
  std::u16string op = b->op().source();
  if (op == u"=") {
    return EvalSimpleAssignment(e, b->lhs(), b->rhs());
  } else if (op == u"*" || op == u"/" || op == u"%" || op == u"-") {
    return EvalArithmeticOperator(e, op, b->lhs(), b->rhs());
  } else if (op == u"+") {
    return EvalAddOperator(e, b->lhs(), b->rhs());
  } else if (op == u"<<" || op == u">>" || op == u">>>") {
    return EvalBitwiseShiftOperator(e, op, b->lhs(), b->rhs());
  } else if (op == u"<" || op == u">" || op == u"<=" || op == u">=" ||
             op == u"instanceof" || op == u"in") {
    return EvalRelationalOperator(e, op, b->lhs(), b->rhs());
  } else if (op == u"==" || op == u"!=" || op == u"===" || op == u"!==") {
    return EvalEqualityOperator(e, op, b->lhs(), b->rhs());
  } else if (op == u"&" || op == u"^" || op == u"|") {
    return EvalBitwiseOperator(e, op, b->lhs(), b->rhs());
  } else if (op == u"&&" || op == u"||") {
    return EvalLogicalOperator(e, op, b->lhs(), b->rhs());
  }
  assert(false);
}

// 11.13.1 Simple Assignment ( = )
JSValue* EvalSimpleAssignment(Error* e, AST* lhs, AST* rhs) {
  JSValue* lref = EvalLeftHandSideExpression(e, lhs);
  if (!e->IsOk()) return nullptr;
  JSValue* rref = EvalExpression(e, rhs);
  if (!e->IsOk()) return nullptr;
  JSValue* rval = GetValue(e, rref);
  if (!e->IsOk()) return nullptr;
  if (lref->type() == JSValue::JS_REF) {
    Reference* ref = static_cast<Reference*>(lref);
    // NOTE in 11.13.1.
    // TODO(zhuzilin) not sure how to implement the type error part of the note.
    if (ref->IsStrictReference() && ref->IsUnresolvableReference()) {
      *e = *Error::ReferenceError();
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

// 11.5 Multiplicative Operators
JSValue* EvalArithmeticOperator(Error* e, std::u16string op, AST* lhs, AST* rhs) {
  JSValue* lref = EvalExpression(e, lhs);
  if (!e->IsOk()) return nullptr;
  JSValue* lval = GetValue(e, lref);
  if (!e->IsOk()) return nullptr;
  JSValue* rref = EvalExpression(e, rhs);
  if (!e->IsOk()) return nullptr;
  JSValue* rval = GetValue(e, rref);
  if (!e->IsOk()) return nullptr;
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
JSValue* EvalAddOperator(Error* e, AST* lhs, AST* rhs) {
  JSValue* lref = EvalExpression(e, lhs);
  if (!e->IsOk()) return nullptr;
  JSValue* lval = GetValue(e, lref);
  if (!e->IsOk()) return nullptr;
  JSValue* rref = EvalExpression(e, rhs);
  if (!e->IsOk()) return nullptr;
  JSValue* rval = GetValue(e, rref);
  if (!e->IsOk()) return nullptr;
  JSValue* lprim = ToPrimitive(e, lval, u"");
  if (!e->IsOk()) return nullptr;
  JSValue* rprim = ToPrimitive(e, rval, u"");
  if (!e->IsOk()) return nullptr;
  // TODO(zhuzilin) Add test when StringObject is added.
  if (lprim->IsString() && rprim->IsString()) {
    return new String(ToString(e, lprim) + ToString(e, rprim));
  }

  double lnum = ToNumber(e, lprim);
  if (!e->IsOk()) return nullptr;
  double rnum = ToNumber(e, rprim);
  if (!e->IsOk()) return nullptr;

  return new Number(lnum + rnum);
}

// 11.7 Bitwise Shift Operators
JSValue* EvalBitwiseShiftOperator(Error* e, std::u16string op, AST* lhs, AST* rhs) {
  JSValue* lref = EvalExpression(e, lhs);
  if (!e->IsOk()) return nullptr;
  JSValue* lval = GetValue(e, lref);
  if (!e->IsOk()) return nullptr;
  JSValue* rref = EvalExpression(e, rhs);
  if (!e->IsOk()) return nullptr;
  JSValue* rval = GetValue(e, rref);
  int32_t lnum = ToInt32(e, lval);
  if (!e->IsOk()) return nullptr;
  uint32_t rnum = ToUint32(e, rval);
  if (!e->IsOk()) return nullptr;
  uint32_t shift_count = rnum & 0x1F;
  if (op == u"<<") {
    return new Number(lnum << shift_count);
  } else if (op == u">>") {
    return new Number(lnum << shift_count);
  } else if (op == u">>>") {
    uint32_t lnum = ToUint32(e, lval);
    return new Number(lnum >> rnum);
  }
  assert(false);
}

// 11.8 Relational Operators
JSValue* EvalRelationalOperator(Error* e, std::u16string op, AST* lhs, AST* rhs) {
  JSValue* lref = EvalExpression(e, lhs);
  if (!e->IsOk()) return nullptr;
  JSValue* lval = GetValue(e, lref);
  if (!e->IsOk()) return nullptr;
  JSValue* rref = EvalExpression(e, rhs);
  if (!e->IsOk()) return nullptr;
  JSValue* rval = GetValue(e, rref);
  if (!e->IsOk()) return nullptr;
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
    return Bool::Wrap(static_cast<Bool*>(r)->data());
  } else if (op == u">=") {
    JSValue* r = LessThan(e, lval, rval);
    if (!e->IsOk()) return nullptr;
    if (r->IsUndefined())
      return Bool::True();
    return Bool::Wrap(static_cast<Bool*>(r)->data());
  } else if (op == u"instanceof") {
    if (!rval->IsObject()) {
      *e = *Error::TypeError();
      return nullptr;
    }
    JSObject* obj = static_cast<JSObject*>(rval);
    if (!obj->IsFunction()) {  // only FunctionObject has [[HasInstance]]
      *e = *Error::TypeError();
      return nullptr;
    }
    return Bool::Wrap(obj->HasInstance(e, lval));
  } else if (op == u"in") {
    if (!rval->IsObject()) {
      *e = *Error::TypeError();
      return nullptr;
    }
    JSObject* obj = static_cast<JSObject*>(rval);
    return Bool::Wrap(obj->HasProperty(ToString(e, lval)));
  }
  assert(false);
}

// 11.9 Equality Operators
JSValue* EvalEqualityOperator(Error* e, std::u16string op, AST* lhs, AST* rhs) {
  JSValue* lref = EvalExpression(e, lhs);
  if (!e->IsOk()) return nullptr;
  JSValue* lval = GetValue(e, lref);
  if (!e->IsOk()) return nullptr;
  JSValue* rref = EvalExpression(e, rhs);
  if (!e->IsOk()) return nullptr;
  JSValue* rval = GetValue(e, rref);
  if (!e->IsOk()) return nullptr;
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
JSValue* EvalBitwiseOperator(Error* e, std::u16string op, AST* lhs, AST* rhs) {
  JSValue* lref = EvalExpression(e, lhs);
  if (!e->IsOk()) return nullptr;
  JSValue* lval = GetValue(e, lref);
  if (!e->IsOk()) return nullptr;
  JSValue* rref = EvalExpression(e, rhs);
  if (!e->IsOk()) return nullptr;
  JSValue* rval = GetValue(e, rref);
  if (!e->IsOk()) return nullptr;
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

  JSValue* base = EvalExpression(e, lhs->base());
  for (size_t i = 0; i < base_offset; i++) {
    if (base == nullptr)
      return base;
    auto pair = lhs->order()[i];
    switch (pair.second) {
      case LHS::PostfixType::CALL: {
        auto args = lhs->args_list()[pair.first];
        auto arg_list = EvalArgumentsList(e, args);
        if (!e->IsOk())
          return nullptr;
        base = EvalCallExpression(e, base, arg_list);
        if (!e->IsOk())
          return nullptr;
        break;
      }
      case LHS::PostfixType::INDEX: {
        auto index = lhs->index_list()[pair.first];
        base = EvalIndexExpression(e, base, index);
        break;
      }
      case LHS::PostfixType::PROP: {
        auto prop = lhs->prop_name_list()[pair.first];
        base = EvalIndexExpression(e, base, prop);
        break;
      }
      default:
        assert(false);
        break;
    }

  }
  // NewExpression
  for (size_t i = 0; i < new_count; i++) {
    base = GetValue(e, base);
    if (!e->IsOk())
      return nullptr;
    if (!base->IsConstructor()) {
      *e = *Error::TypeError();
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
    base = constructor->Construct(e, {});
    if (!e->IsOk())
      return nullptr;
  }

  return base;
}

std::vector<JSValue*> EvalArgumentsList(Error* e, Arguments* ast) {
  log::PrintSource("enter EvalArgumentsList");
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
  log::PrintSource("enter EvalCallExpression");
  JSValue* val = GetValue(e, ref);
  if (!e->IsOk())
    return nullptr;
  if (!val->IsObject()) {  // 4
    *e = *Error::TypeError();
    return nullptr;
  }
  auto obj = static_cast<JSObject*>(val);
  if (!obj->IsCallable()) {  // 5
    *e = *Error::TypeError();
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
  return obj->Call(e, this_value, arg_list);
}

// 11.2.1 Property Accessors
JSValue* EvalIndexExpression(Error* e, JSValue* base_ref, std::u16string identifier_name) {
  JSValue* base_value = GetValue(e, base_ref);
  if (!e->IsOk())
    return nullptr;
  base_value->CheckObjectCoercible(e);
  if (!e->IsOk())
    return nullptr;
  bool strict = ExecutionContextStack::Global()->Top().strict();
  return new Reference(base_value, identifier_name, strict);
}

JSValue* EvalIndexExpression(Error* e, JSValue* base_ref, AST* expr) {
  JSValue* property_name_ref = EvalExpression(e, expr);
  if (!e->IsOk())
    return nullptr;
  JSValue* property_name_value = GetValue(e, property_name_ref);
  if (!e->IsOk())
    return nullptr;
  std::u16string property_name_str = ToString(e, property_name_value);
  if (!e->IsOk())
    return nullptr;
  return EvalIndexExpression(e, base_ref, property_name_str);
}

}  // namespace es

#endif  // ES_EVALUATOR_H