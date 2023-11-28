/* Adam Mariano
	Implementation of Recursive-Descent Parser for a 
	Simple Pasacal-Like Language
 * Programming Assignment 2 - CS280
 * Fall 2023
*/

#include "parser.h"

//keeps track of defined variables; string = name, bool = if initialized
map<string, bool> defVar;
map<string, Token> SymTable;

namespace Parser {
	bool pushed_back = false;
	LexItem	pushed_token;

	//Token LookAhead
	static LexItem GetNextToken(istream& in, int& line) {
		if( pushed_back ) {
			pushed_back = false;
			return pushed_token;
		}
		return getNextToken(in, line);
	}

	static void PushBackToken(LexItem & t) {
		if( pushed_back ) {
			abort();
		}
		pushed_back = true;
		pushed_token = t;	
	}

}

//counting errors
static int error_count = 0;

int ErrCount()
{
    return error_count;
}

//display error messages
void ParseError(int line, string msg)
{
	++error_count;
	cout << line << ": " << msg << endl;
}

//Prog ::= PROGRAM IDENT ; DeclPart CompoundStmt .
bool Prog(istream& in, int& line)
{
	LexItem t;

	t = Parser::GetNextToken(in, line); //Check for PROGRAM keyword
	if (t != PROGRAM)
	{
		ParseError(line, "Missing PROGRAM Keyword.");
		return false;
	}
	t = Parser::GetNextToken(in, line); //Check for Program Name 
	if (t != IDENT){
		ParseError(line, "Missing Program Name.");
		return false;
	}
	t = Parser::GetNextToken(in, line); //check for the following semicolon
	if (t != SEMICOL){
		ParseError(line, "Missing Semicolon.");
		return false;
	}

	bool declP = DeclPart(in, line); 
	if (!declP)
	{
		ParseError(line, "Incorrect Declaration Section.");
		return false;
	}
	bool CStmt = CompoundStmt(in, line);
	if (!CStmt)
	{
		ParseError(line, "Missing Compound Statement.");
		return false;
	}
	t = Parser::GetNextToken(in, line); //Check for Termination dot
	if(t != DOT){
		ParseError(line, "Missing Final Termination Dot.");
		return false;
	}
	return true; //Parse Success
}//End of Prog

//DeclPart ::= VAR DeclStmt; {	DeclStmt ; }
bool DeclPart(istream& in, int& line)
{
	//VAR
	LexItem t = Parser::GetNextToken(in, line);
	if(t != VAR)
	{
		ParseError(line, "Missing Var Declaration Section.");
		return false;
	}

	//DeclStmt
	bool status = DeclStmt(in, line);
	while (status)
	{
		t = Parser::GetNextToken(in, line);
		if (t == BEGIN)
		{
			Parser::PushBackToken(t);
			return true;
		}
		if (t != SEMICOL)
		{
			ParseError(line, "Missing Semicolon Declaration Section.");
			return false;
		}

		status = DeclStmt(in, line);
	}

	if(!status)
	{
		ParseError(line, "Unrecognizeable Declaration Section.");
		return false;
	}

	return true;
}//End of DeclPart

//DeclStmt ::= IDENT {, IDENT } : Type [:= Expr]
bool DeclStmt(istream& in, int& line)
{
	bool status = false;
	LexItem t = Parser::GetNextToken(in, line);
	if (t == BEGIN)
	{
		Parser::PushBackToken(t);
		return true;
	}
	else if (t != IDENT)
	{
		ParseError(line, "Unrecognizeable Variable Declaration Statement.");
		return false;
	}
	else if (defVar[t.GetLexeme()])
	{
		ParseError(line, "Variable Redefinition.");
		ParseError(line, "Incorrect identifiers list in Declaration Statement.");
		return false;
	}
	defVar[t.GetLexeme()] = true;
	bool rep = true;
	while (rep)
	{
		t = Parser::GetNextToken(in, line);

		if (t != COMMA)
		{
			Parser::PushBackToken(t);
			rep = false;
			break;
		}
		t = Parser::GetNextToken(in, line);
		if (t != IDENT)
		{
			ParseError(line, "Unrecognizeable Variable Declaration Statement.");
			return false;
		}
		else if (defVar[t.GetLexeme()])
		{
			ParseError(line, "Variable Redefinition.");
			ParseError(line, "Incorrect identifiers list in Declaration Statement.");
			return false;
		}
		defVar[t.GetLexeme()] = true;
	}
	
	t = Parser::GetNextToken(in, line);
	if (t != COLON)
	{
		ParseError(line, "Missing Colon in Declaration Statement.");
		return false;
	}

	t = Parser::GetNextToken(in, line);
	if (t != INTEGER && t != REAL && t != BOOLEAN && t != STRING)
	{
		ParseError(line, "Unrecognized Type in Declaration Statement.");
		return false;
	}

	t = Parser::GetNextToken(in, line);
	if (t == ASSOP)
	{
		status = Expr(in, line);
		if (!status)
		{
			ParseError(line, "Unrecognized Expression in Declaration Statement.");
			return false;
		}
		return true;
	}
	else{
		Parser::PushBackToken(t);
		return true;
	}
	return status;
}//End of DeclStmt

//Stmt ::= SimpleStmt | StructuredStmt
bool Stmt(istream& in, int& line)
{
	bool status = false;
	LexItem t = Parser::GetNextToken(in, line);
	if (t == BEGIN || t == IF)
	{ //StructuredStmt
		Parser::PushBackToken(t);
		status = StructuredStmt(in, line);
		if (!status)
		{
			ParseError(line, "Invalid StructuredStmt.");
			return false;
		}
		return status;
	}
	else if (t == IDENT || t == WRITE || t == WRITELN)
	{ //SimpleStmt
		Parser::PushBackToken(t);
		status = SimpleStmt(in, line);
		if (!status)
		{
			ParseError(line, "Invalid SimpleStmt.");
			return false;
		}
		return status;
	}
	return true;
}//End of Stmt

//StructuredStmt ::= IfStmt | CompoundStmt
bool StructuredStmt(istream& in, int& line)
{
	bool status = false;
	LexItem t = Parser::GetNextToken(in, line);
	if (t == IF)
	{//IfStmt
		Parser::PushBackToken(t);
		status = IfStmt(in, line);
		if (!status)
		{
			ParseError(line, "Error IfStmt.");
			return false;
		}
		return status;
	}
	else if (t == BEGIN)
	{//CompoundStmt
		Parser::PushBackToken(t);
		status = CompoundStmt(in, line);
		if (!status)
		{
			ParseError(line, "Error CompoundStmt.");
			return false;
		}
		return status;
	}
	Parser::PushBackToken(t);
	return false;
}//End of StructuredStmt

//CompoundStmt ::= BEGIN Stmt {; Stmt } END
bool CompoundStmt(istream& in, int& line)
{
	LexItem t = Parser::GetNextToken(in, line);
	if (t != BEGIN)
	{
		ParseError(line, "Unrecognized Compound Statement.");
		return false;
	}

	bool status = Stmt(in, line);
	if (!status)
	{
		ParseError(line, "Incorrect statement in Compound Statement.");
		return false;
	}

	bool rep = true;
	while(rep)
	{
		t = Parser::GetNextToken(in, line);
		if (t != SEMICOL)
		{
			Parser::PushBackToken(t);
			rep = false;
			break;
		}

		status = Stmt(in, line);
		if (!status)
		{
			ParseError(line, "Incorrect statement in Compound Statement.");
			return false;
		}
	}

	t = Parser::GetNextToken(in, line);
	if (t != END)
	{
		ParseError(line, "Missing END keyword in Compound Statement.");
		return false;
	}
	
	return true;
}//End of CompoundStmt

//SimpleStmt ::= AssignStmt | WriteLnStmt | WriteStmt
bool SimpleStmt(istream& in, int& line)
{
	bool status = false;
	LexItem t = Parser::GetNextToken(in , line);

	if (t == IDENT)
	{//AssignStmt
		Parser::PushBackToken(t);
		status = AssignStmt(in, line);
		if (!status)
		{
			ParseError(line, "Error AssignStmt.");
			return false;
		}
		return status;
	}

	else if (t == WRITE)
	{
		//Parser::PushBackToken(t);
		status = WriteStmt(in, line);
		if (!status)
		{
			ParseError(line, "Error WriteStmt.");
			return false;
		}
		return status;
	}

	else if (t == WRITELN)
	{
		//Parser::PushBackToken(t);
		status = WriteLnStmt(in, line);
		if (!status)
		{
			ParseError(line, "Error WriteLnStmt.");
			return false;
		}
		return status;
	}
	Parser::PushBackToken(t);
	return status;
}//End of SimpleStmt

//WriteStmt ::= WRITE (ExprList)
bool WriteStmt(istream& in, int& line)
{
	LexItem t;
	
	t = Parser::GetNextToken(in, line);
	if( t != LPAREN ) {
		
		ParseError(line, "Missing Left Parenthesis.");
		return false;
	}
	
	bool status = ExprList(in, line);
	
	if( !status ) {
		ParseError(line, "Missing expression list for WriteLn statement.");
		return false;
	}
	
	t = Parser::GetNextToken(in, line);
	if(t != RPAREN ) {
		
		ParseError(line, "Missing Right Parenthesis.");
		return false;
	}
	//Evaluate: print out the list of expressions values

	return status;
}//End of Write Stmt

//IfStmt ::= IF Expr THEN Stmt [ ELSE Stmt ]
bool IfStmt(istream& in, int& line)
{
	LexItem t = Parser::GetNextToken(in, line);
	if (t != IF)
	{
		ParseError(line, "Missing IF keyword.");
		return false;
	}

	bool status = Expr(in, line);
	if (!status)
	{
		ParseError(line, "Invalid Expr in IfStmt.");
		return false;
	}

	t = Parser::GetNextToken(in, line);
	if (t != THEN)
	{
		ParseError(line, "Missing THEN keyword.");
		return false;
	}

	status = Stmt(in, line);
	if (!status)
	{
		ParseError(line, "Invalid Stmt in IfStmt.");
		return false;
	}

	t = Parser::GetNextToken(in, line);
	if (t == ELSE)
	{
		status = Stmt(in, line);
		if (!status)
		{
			ParseError(line, "Invalid Stmt in IfStmt Else.");
			return false;
		}
		return true;
	}
	else
	{
		Parser::PushBackToken(t);
		return true;
	}
	return status;
}

//AssignStmt ::= Var := Expr
bool AssignStmt(istream& in, int& line)
{
	bool status = Var(in, line);
	if (!status)
	{
		ParseError(line, "Invalid Variable in Assignment.");
		return false;
	}

	LexItem t = Parser::GetNextToken(in, line);
	if (t != ASSOP)
	{
		ParseError(line, "Missing Assignment operator.");
		return false;
	}

	status = Expr(in, line);
	if (!status)
	{
		ParseError(line, "Invalid Expression in Assignment.");
		return false;
	}

	return status;
}

//Var ::= IDENT
bool Var(istream& in, int& line)
{
	LexItem t = Parser::GetNextToken(in, line);
	if (t == IDENT)
	{
		if (!(defVar[t.GetLexeme()]))
		{
			ParseError(line, "Undeclared Variable.");
			return false;
		}
		return true;
	}
	else
	{
		ParseError(line, "Error in Var.");
		return false;
	}
	return false;
}

//Expr ::= LogOrExpr ::= LogAndExpr { OR LogAndExpr }
bool Expr(istream& in, int& line)
{
	bool status = LogANDExpr(in, line);
	if (!status)
	{
		ParseError(line, "Unrecognized OR Expression.");
		return false;
	}

	LexItem t = Parser::GetNextToken(in, line);
	if (t == OR)
	{
		status = Expr(in, line);
	}
	else{
		Parser::PushBackToken(t);
		return true;
	}
	return status;
}//End of Expr

//LogAndExpr ::= RelExpr {AND RelExpr }
bool LogANDExpr(istream& in, int& line)
{
	bool status = RelExpr(in, line);
	if (!status)
	{
		ParseError(line, "Unrecognized AND Expression.");
		return false;
	}

	LexItem t = Parser::GetNextToken(in, line);
	if (t == AND)
	{
		status = LogANDExpr(in, line);
	}
	else{
		Parser::PushBackToken(t);
		return true;
	}
	return status;
}//End of LogAndExpr

//RelExpr ::= SimpleExpr [ ( = | < | > ) SimpleExpr ]
bool RelExpr(istream& in, int& line)
{
	bool status = SimpleExpr(in, line);
	if (!status)
	{
		ParseError(line, "Error RelExpr.");
		return false;
	}

	LexItem t = Parser::GetNextToken(in, line);
	if (t == EQ || t == LTHAN || t == GTHAN)
	{
		status = SimpleExpr(in, line);
		if (!status)
		{
			ParseError(line, "Incomplete Relation Expression.");
			return false;
		}
		return true;
	}
	else
	{
		Parser::PushBackToken(t);
	}
	return true;
}// End of RelExpr

//SimpleExpr :: Term { ( + | - ) Term }
bool SimpleExpr(istream& in, int& line)
{
	bool status = Term(in, line);
	if (!status)
	{
		ParseError(line, "Unrecognized Term.");
		return false;
	}

	LexItem t = Parser::GetNextToken(in, line);
	if (t == PLUS || t == MINUS)
	{
		status = Term(in, line);
		if (!status)
		{
			ParseError(line, "Incomplete Term.");
			return false;
		}
		return true;
	}
	else
	{
		Parser::PushBackToken(t);
	}
	return true;
}//End of SimpleExpr

//Term ::= SFactor { ( * | / | DIV | MOD ) SFactor }
bool Term(istream& in, int& line)
{
	bool status = SFactor(in, line);
	if (!status)
	{
		ParseError(line, "Unrecognized Signed Factor.");
		return false;
	}

	LexItem t = Parser::GetNextToken(in, line);
	if (t == MULT || t == DIV || t == IDIV || t == MOD)
	{
		status = Term(in, line);
	}
	else
	{
		Parser::PushBackToken(t);
		return true;
	}
	return true;
}//End of Term

//SFactor ::= [( - | + | NOT )] Factor
bool SFactor(istream& in, int& line)
{
	bool status;
	LexItem t = Parser::GetNextToken(in, line);
	int sign = 1;
	if (t == MINUS || t == NOT)
	{
		sign = -1;
	}
	else if (t != PLUS)
	{
		Parser::PushBackToken(t);
	}

	status = Factor(in, line, sign);
	if (!status)
	{
		ParseError(line, "Unrecognized Factor.");
		return false;
	}
	return true;
}//End of SFactor

//Factor ::= IDENT | ICONST | RCONST | SCONST | BCONST | (Expr)
bool Factor(istream& in, int& line, int sign)
{
	bool status;
	LexItem t = Parser::GetNextToken(in, line);
	if (t == LPAREN)
	{
		status = Expr(in, line);
		if (!status)
		{
			ParseError(line, "Invalid Expression.");
			return false;
		}

		t = Parser::GetNextToken(in, line);
		if (t != RPAREN)
		{
			ParseError(line, "Missing Right Parenthesis.");
			return false;
		}
		return true;
	}
	else if (t != IDENT && t != ICONST && t != RCONST && t != SCONST && t != BCONST)
	{
		ParseError(line, "Invalid Factor.");
		return false;
	}
	else if (t == IDENT && !defVar[t.GetLexeme()])
	{
		ParseError(line, "Undeclared Variable in Factor.");
		return false;
	}
	else
	{
		return true;
	}
}//End of Factor

//WriteLnStmt ::= writeln (ExprList) 
bool WriteLnStmt(istream& in, int& line) {
	LexItem t;
	//cout << "in WriteStmt" << endl;
	
	t = Parser::GetNextToken(in, line);
	if( t != LPAREN ) {
		
		ParseError(line, "Missing Left Parenthesis");
		return false;
	}
	
	bool ex = ExprList(in, line);
	
	if( !ex ) {
		ParseError(line, "Missing expression list for WriteLn statement");
		return false;
	}
	
	t = Parser::GetNextToken(in, line);
	if(t != RPAREN ) {
		
		ParseError(line, "Missing Right Parenthesis");
		return false;
	}
	//Evaluate: print out the list of expressions values

	return ex;
}//End of WriteLnStmt


//ExprList:= Expr {,Expr}
bool ExprList(istream& in, int& line) {
	bool status = false;
	//cout << "in ExprList and before calling Expr" << endl;
	status = Expr(in, line);
	if(!status){
		ParseError(line, "Missing Expression");
		return false;
	}
	
	LexItem tok = Parser::GetNextToken(in, line);
	
	if (tok == COMMA) {
		//cout << "before calling ExprList" << endl;
		status = ExprList(in, line);
		//cout << "after calling ExprList" << endl;
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else{
		Parser::PushBackToken(tok);
		return true;
	}
	return status;
}//ExprList




