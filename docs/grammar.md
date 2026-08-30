# Grammar (implemented subset)

Notation: `*` zero or more, `?` optional.

```ebnf
program       ::= declaration* EOF ;
declaration   ::= modifiers* ( variable | function | class | interface ) | statement ;
variable      ::= ("let" | "set") IDENT (":" type)? ("=" expression)? ";" ;
function      ::= "func" IDENT "(" parameters? ")" (":" type)? block ;
parameters    ::= IDENT ":" type ("," IDENT ":" type)* ;
class         ::= "class" IDENT ("extends" IDENT)? "{" class-member* "}" ;
class-member  ::= modifiers* ("init" | "func" IDENT) "(" parameters? ")" (":" type)? block
                | modifiers* IDENT ":" type ("=" expression)? ";" ;
interface     ::= "intf" IDENT "{" interface-member* "}" ;
block         ::= "{" (declaration | statement | expression ";")* expression? "}" ;
statement     ::= block | if-statement | while | loop | break | continue | return | expression ";" ;
if-statement  ::= "if" "(" expression ")" block ("else" (if-statement | block))? ;
while         ::= "while" "(" expression ")" block ;
loop          ::= (IDENT ":")? "loop" ("(" variable-or-expression ";" expression? ";" expression? ")")? block ;
break         ::= "break" IDENT? ";" ; continue ::= "continue" IDENT? ";" ;
return        ::= "return" expression? ";" ;
type          ::= IDENT | primitive ("?")? ("|" type)* ;
expression    ::= assignment ;
assignment    ::= logic-or ("=" assignment)? ;
primary       ::= literal | IDENT | "self" | "super" | "new" IDENT "(" arguments? ")"
                | "(" expression ")" | object | if-expression | match ;
match         ::= "match" "(" expression ")" "{" (literal | "_") "=>" expression ";"* "}" ;
```

Precedence from low to high is assignment, `||`, `&&`, equality, comparison, `+/-`, `*/%`, unary, call/member.
