#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ab.h>
#include "color.h"

static char LITERAL_PRINT[] = "print";

static char* ReadEntireFile(const char* FileName)
{
    char *Result = 0;
    
    FILE *File = fopen(FileName, "r");
    if(File)
    {
        fseek(File, 0, SEEK_END);
        size_t FileSize = ftell(File);
        fseek(File, 0, SEEK_SET);

        Result = (char *)malloc(FileSize + 1);
        fread(Result, FileSize, 1, File);
        Result[FileSize] = 0;
        
        fclose(File);
    }

    return(Result);
}

enum token_type {
    Token_Unknown,
    
    Token_OpenParen,    
    Token_CloseParen,    
    Token_Colon,    
    // Token_Semicolon,
    Token_Number,
    Token_Asterisk,    
    Token_OpenBracket,    
    Token_CloseBracket,    
    Token_OpenBrace,    
    Token_CloseBrace,

    Token_String,    
    Token_Identifier,

    Token_EndOfStream,
};
enum var_type {
	Var_Number, // All Number's longs?
};
enum operator_type {
	Operator_Add,
	Operator_Sub,
	Operator_Mul,
	Operator_Div
};
struct cursor {
	char *At;
};
struct token {
	token_type Type;
	int TextLength;
	char *Text;
};
bool IsEndOfLine(char C) {
	bool Result = ((C == '\n') || (C == '\r'));
	return(Result);
}
bool IsWhitespace(char C) {
	bool Result = ((C == ' ') || (C == '\t') || (C == '\v') || (C == '\f') || IsEndOfLine(C));

	return(Result);
}
bool IsAlpha(char C){
	bool Result = (((C >= 'a') && (C <= 'z')) || ((C >= 'A') && (C <= 'Z')));
	return(Result);
}
bool IsNumber(char C) {
	bool Result = ((C >= '0') && (C <= '9'));
	return(Result);
}
void EatAllWhitespace(cursor* Cursor) {
	for (;;) {
		if (IsWhitespace(*Cursor->At)) {
			++Cursor->At;
		} else if (*Cursor->At == '#') {
			++Cursor->At;
			while (*Cursor->At && !IsEndOfLine(*Cursor->At)) {
				++Cursor->At;
			}
			++Cursor->At;
		} else break;
	}
}
void BackToken(cursor* Cursor, token Token) {
	Cursor->At = Token.Text;
}
token GetToken(cursor* Cursor) {
	EatAllWhitespace(Cursor);

	token Token = {};
	Token.TextLength = 1;
	Token.Text = Cursor->At;
	char c = *Cursor->At;
	++Cursor->At;
	switch(c) {
		case '\0': { Token.Type = Token_EndOfStream; } break;

		case '(': { Token.Type = Token_OpenParen; } break;
		case ')': { Token.Type = Token_CloseParen; } break;
		case ':': { Token.Type = Token_Colon; } break;
		// case ';': { Token.Type = Token_Semicolon; } break;
		case '*': { Token.Type = Token_Asterisk; } break;
		case '[': { Token.Type = Token_OpenBracket; } break;
		case ']': { Token.Type = Token_CloseBracket; } break;
		case '{': { Token.Type = Token_OpenBrace; } break;
		case '}': { Token.Type = Token_CloseBrace; } break;

		default:
		{
			if (IsAlpha(c)) {
				Token.Type = Token_Identifier;
				while (IsAlpha(*Cursor->At) || IsNumber(*Cursor->At) || *Cursor->At == '_') {
					++Cursor->At;
				}
			} else if (IsNumber(c)) {
				Token.Type = Token_Number;
				while (IsNumber(*Cursor->At)) {
					++Cursor->At;
				}
			} else {
				Token.Type = Token_Unknown;
			}
			Token.TextLength = Cursor->At - Token.Text;
		}
	}

	return Token;
}
bool RequireToken(cursor *Cursor, token_type DesiredType) {
    token Token = GetToken(Cursor);
    bool Result = (Token.Type == DesiredType);
    return Result;
}
bool TokenEquals(token Token, char *Match) {
    char *At = Match;
    for(int Index = 0; Index < Token.TextLength; ++Index, ++At) {
        if((*At == 0) || (Token.Text[Index] != *At)) {
        	// printf("%c-%c//\n", Token.Text[Index], *At);
            return false;
        }
    }
    // printf("\n");

    // bool Result = (*At == 0);
    // printf("%c\n", *At);
    // return Result;
    return true;
}
struct var {
	token Identifier;
	var_type Type;
	union {
		double v_Number;
	};
};
struct operation {
	var* Dest;
	var* SourceA;
	var* SourceB;
	operator_type Type;
};
struct scope {
	array* Variables;
	// scope* Scopes;
};
var* GetVarByName(scope* Scope, char* Name) {
	for (int i = 0; i < Scope->Variables->size; i++) {
		var* Var = (var*) array_get(Scope->Variables, i);
		if (TokenEquals(Var->Identifier, Name)) {
			return Var;
		}
	}
	return NULL;
}

void CarryOperation(operation* Op) {
	assert(Op->SourceA->Type == Op->SourceB->Type);
	if (Op->SourceA->Type == Var_Number) {
		double ValueA = Op->SourceA->v_Number;
		double ValueB = Op->SourceB->v_Number;
		switch(Op->Type) {
		case Operator_Add: Op->Dest->v_Number = ValueA + ValueB; break;
		case Operator_Sub: Op->Dest->v_Number = ValueA - ValueB; break;
		case Operator_Mul: Op->Dest->v_Number = ValueA * ValueB; break;
		case Operator_Div: Op->Dest->v_Number = ValueA / ValueB; break;
		}
	}
}

var* ParseExpression(scope* Scope, cursor* Cursor) {
	array* Stack = array_new();
	for (;;) {
		token Token = GetToken(Cursor);
		var* Var = (var*) malloc(sizeof(var));
		if (Token.Type == Token_Number) {
			Var->v_Number = atof(Token.Text);
			// printf("%f\n", Var->v_Number);
		} else if (Token.Type == Token_Identifier) {
			// printf("Token %.*s\n", Token.TextLength, Token.Text);
			var* PreviousVariable = GetVarByName(Scope, Token.Text);
			// printf("%d\n", PreviousVariable != NULL);
			if (PreviousVariable) {
				// printf("yep\n");
				Var->v_Number = PreviousVariable->v_Number;
			} else {
				// printf("Back\n");
				BackToken(Cursor, Token);
				return (var*) array_remove_last(Stack);
			}
		} else if (*Token.Text == '+' || *Token.Text == '-' || *Token.Text == '*' || *Token.Text == '/') {
			assert(Stack->size >= 2);
			var* Stack1 = (var*) array_remove_last(Stack);
			var* Stack2 = (var*) array_remove_last(Stack);
			operation Op = {};
			Op.Dest = Var;
			// Reversed because LIFO.
			Op.SourceA = Stack2;
			Op.SourceB = Stack1;
			switch(*Token.Text) {
			case '+': Op.Type = Operator_Add; break;
			case '-': Op.Type = Operator_Sub; break;
			case '*': Op.Type = Operator_Mul; break;
			case '/': Op.Type = Operator_Div; break;
			}
			CarryOperation(&Op);
			// printf("%f %f\n", Stack1->v_Number, Stack2->v_Number);
			// Var->v_Number = Stack1->v_Number + Stack2->v_Number;
		} else {
			printf(KRED "Syntax error" KNRM ": Unexpected identifier %.*s\n", Token.TextLength, Token.Text);
			exit(0);
		}
		array_add(Stack, (void*) Var);
		// if (Token.Type == Token_Number) return Var;
	}
}
bool ParseVariable(scope* Scope, cursor* Cursor, token Name) {
	if (!RequireToken(Cursor, Token_Colon)) return false;
	token Equals = GetToken(Cursor);
	if (*Equals.Text != '=') return false;
	var* Var = (var*) malloc(sizeof(var));
	Var->Identifier = Name;
	var* Result = ParseExpression(Scope, Cursor);
	Var->v_Number = Result->v_Number;
	// printf("%f\n", Result->v_Number);
	array_add(Scope->Variables, Var);
	// if (Value.Type == Token_Number) {
	// 	Var->v_Number = atof(Value.Text);
	// 	// printf("%.*s\n", Value.TextLength, Value.Text);
	// } else if (Value.Type == Token_Identifier) {
	// 	token NextValue = GetToken(Cursor);
	// 	token OperationToken = GetToken(Cursor);
	// 	operation Operation = {};
	// 	if (*OperationToken.Text == '+')
	// 		Operation.Type = Operator_Add;
	// 	// printf("%.*s\n", Value.TextLength, Value.Text);
	// 	// printf("%.*s\n", NextValue.TextLength, NextValue.Text);
	// 	Operation.A = GetVarByName(Scope, Value.Text);
	// 	Operation.B = GetVarByName(Scope, NextValue.Text);
	// 	Var->v_Number = Operation.A->v_Number + Operation.B->v_Number;
	// } else {
	// 	free(Var);
	// 	return false;
	// }
	return true;
}
int main(int argc, char** argv) {
	setvbuf(stdout, NULL, _IONBF, 0);

	char* Source = ReadEntireFile("test/var_implicit_multi.lx");
	cursor Cursor = {};
	Cursor.At = Source;

	scope Scope;
	Scope.Variables = array_new();
	for (;;) {
		token Token = GetToken(&Cursor);
		if (Token.Type == Token_EndOfStream) {
			break;
		} else if (Token.Type == Token_Identifier) {
			if (TokenEquals(Token, LITERAL_PRINT)) {
				if (!RequireToken(&Cursor, Token_OpenParen)) {
					printf(KRED "Syntax error" KNRM ": Missing open paren\n");
					exit(0);	
				}
				var* Var = GetVarByName(&Scope, GetToken(&Cursor).Text);
				if (!RequireToken(&Cursor, Token_CloseParen)) {
					printf(KRED "Syntax error" KNRM ": Missing close paren\n");
					exit(0);	
				}
				printf("%f\n", Var->v_Number);
			} else {
				ParseVariable(&Scope, &Cursor, Token);
			}
		}
		// printf("%d: %.*s\n", Token.Type, Token.TextLength, Token.Text);
	}

	return 0;
}