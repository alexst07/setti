#include <iostream>
#include <gtest/gtest.h>

#include "parser/parser.h"
#include "ast/ast-printer.h"

TEST(Lexer, Check) {
  using namespace setti::internal;

  Lexer l("5 +4* 4+51-(\"oi\" - wef) /\n78+5.86");
  std::cout << "Lexer\n";
  TokenStream ts = l.Scanner();
  Parser p(std::move(ts));
  auto res = p.AstGen();

  if (p.nerrors() == 0) {
    std::cout << "Correct analysis\n";
    AstPrinter visitor;
    visitor.Visit(res.NodePtr());
  } else {
    std::cout << "Parser error analysis\n";
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


