#include "SyntaxHighlighter.h"

SyntaxHighlighter::SyntaxHighlighter(QTextDocument* parent) 
	: QSyntaxHighlighter(parent)
{
}

void SyntaxHighlighter::setDarkColor()
{
	HighlightingRule rule;

#if 1 // 렉이 심하면 빼야함
	QTextCharFormat callinsnFormat;
	callinsnFormat.setForeground(QColor("#842f2f"));

	rule.pattern = QRegularExpression("(call|jmp|jne|je|jae|ja|jbe|jb|jge|jg|jle|jl|jz|jnz)\\s");
	rule.format = callinsnFormat;
	highlightingRules.append(rule);

	QTextCharFormat movinsnFormat;
	movinsnFormat.setForeground(QColor("#664e72").dark());

	rule.pattern = QRegularExpression("(movzx|movsb|movsx|movs|mov)");
	rule.format = movinsnFormat;
	highlightingRules.append(rule);

	QTextCharFormat datainsnFormat;
	datainsnFormat.setForeground(QColor("#5e875b"));

	rule.pattern = QRegularExpression("(pushad|popad|pusha|popa|pushfd|popfd|pushf|popf|push|pop)");
	rule.format = datainsnFormat;
	highlightingRules.append(rule);

	QTextCharFormat ptrinsnFormat;
	ptrinsnFormat.setForeground(QColor("#774343"));

	rule.pattern = QRegularExpression("(qword ptr|dword ptr|word ptr|byte ptr)");
	rule.format = ptrinsnFormat;
	highlightingRules.append(rule);

	QTextCharFormat registerCommentFormat;
	registerCommentFormat.setForeground(QColor("#3d556d"));

	rule.pattern = QRegularExpression("(eax|ebx|ecx|edx|edi|esi|ebp|esp|eip"
		"|rax|rbx|rcx|rdx|rdi|rsi|rbp|rsp|rip|r8|r9|r10|r11|r12|r13|r14|r15)");
	rule.format = registerCommentFormat;
	highlightingRules.append(rule);
#endif

	//
	//
	QTextCharFormat singleLineCommentFormat;
	singleLineCommentFormat.setForeground(QColor("#004405"));

	rule.pattern = QRegularExpression("; [^\n]*");
	rule.format = singleLineCommentFormat;
	highlightingRules.append(rule);

	//
	//
	QTextCharFormat subroutineLineFormat;
	subroutineLineFormat.setForeground(QColor("#005e5e")); //#00ffff
	subroutineLineFormat.setFontWeight(QFont::Bold);

	rule.pattern = QRegularExpression("=========================== subroutine ===========================[^\n]*");
	rule.format = subroutineLineFormat;
	highlightingRules.append(rule);

	//
	//
	QTextCharFormat symCommentFormat;
	symCommentFormat.setForeground(QColor("#00493b"));

	rule.pattern = QRegularExpression(" .sym[^\n]*");
	rule.format = symCommentFormat;
	highlightingRules.append(rule);

	//
	//
	QTextCharFormat breakpointLineFormat;
	breakpointLineFormat.setForeground(QColor("#601414"));

	rule.pattern = QRegularExpression("======= current point");
	rule.format = breakpointLineFormat;
	highlightingRules.append(rule);

	QTextCharFormat exceptionpointLineFormat;
	exceptionpointLineFormat.setForeground(QColor("#601414"));

	rule.pattern = QRegularExpression("======= break point");
	rule.format = exceptionpointLineFormat;
	highlightingRules.append(rule);

	//
	//
	QTextCharFormat xrefCommentFormat;
	xrefCommentFormat.setForeground(QColor("#004f3f"));

	rule.pattern = QRegularExpression(" .xref[^\n]*");
	rule.format = xrefCommentFormat;
	highlightingRules.append(rule);

	//
	//
	QTextCharFormat stringCommentFormat;
	stringCommentFormat.setForeground(QColor("#d17f62"));

	rule.pattern = QRegularExpression(" \"[^\n]*");
	rule.format = stringCommentFormat;
	highlightingRules.append(rule);

	QTextCharFormat lstringCommentFormat;
	lstringCommentFormat.setForeground(QColor("#d17f62"));

	rule.pattern = QRegularExpression(" L\"[^\n]*");
	rule.format = lstringCommentFormat;
	highlightingRules.append(rule);
}

void SyntaxHighlighter::setLightColor()
{
	HighlightingRule rule;

#if 1 // 렉이 심하면 빼야함
	QTextCharFormat callinsnFormat;
	callinsnFormat.setForeground(QColor("#ff5b5b"));

	rule.pattern = QRegularExpression("(call|jmp|jne|je|jae|ja|jbe|jb|jge|jg|jle|jl|jz|jnz)\\s");
	rule.format = callinsnFormat;
	highlightingRules.append(rule);

	QTextCharFormat movinsnFormat;
	movinsnFormat.setForeground(QColor("#e1abfc"));

	rule.pattern = QRegularExpression("(movzx|movsb|movsx|movs|mov)");
	rule.format = movinsnFormat;
	highlightingRules.append(rule);

	QTextCharFormat datainsnFormat;
	datainsnFormat.setForeground(QColor("#affcab"));

	rule.pattern = QRegularExpression("(pushad|popad|pusha|popa|pushfd|popfd|pushf|popf|push|pop)");
	rule.format = datainsnFormat;
	highlightingRules.append(rule);

	QTextCharFormat ptrinsnFormat;
	ptrinsnFormat.setForeground(QColor("#ff8e8e"));

	rule.pattern = QRegularExpression("(qword ptr|dword ptr|word ptr|byte ptr)");
	rule.format = ptrinsnFormat;
	highlightingRules.append(rule);

	QTextCharFormat registerCommentFormat;
	registerCommentFormat.setForeground(QColor("#8ec6ff"));

	rule.pattern = QRegularExpression("(eax|ebx|ecx|edx|edi|esi|ebp|esp|eip"
		"|rax|rbx|rcx|rdx|rdi|rsi|rbp|rsp|rip|r8|r9|r10|r11|r12|r13|r14|r15)");
	rule.format = registerCommentFormat;
	highlightingRules.append(rule);
#endif

	//
	//
	QTextCharFormat singleLineCommentFormat;
	singleLineCommentFormat.setForeground(QColor("#009b0c"));

	rule.pattern = QRegularExpression("; [^\n]*");
	rule.format = singleLineCommentFormat;
	highlightingRules.append(rule);

	//
	//
	QTextCharFormat subroutineLineFormat;
	subroutineLineFormat.setForeground(QColor("#00ffff")); //#00ffff
	subroutineLineFormat.setFontWeight(QFont::Bold);

	rule.pattern = QRegularExpression("=========================== subroutine ===========================[^\n]*");
	rule.format = subroutineLineFormat;
	highlightingRules.append(rule);

	//
	//
	QTextCharFormat symCommentFormat;
	symCommentFormat.setForeground(QColor("#00CEA5"));

	rule.pattern = QRegularExpression(" .sym[^\n]*");
	rule.format = symCommentFormat;
	highlightingRules.append(rule);

	//
	//
	QTextCharFormat breakpointLineFormat;
	breakpointLineFormat.setForeground(QColor("#ff3838"));

	rule.pattern = QRegularExpression("======= current point");
	rule.format = breakpointLineFormat;
	highlightingRules.append(rule);

	QTextCharFormat exceptionpointLineFormat;
	exceptionpointLineFormat.setForeground(QColor("#ff3838"));

	rule.pattern = QRegularExpression("======= break point");
	rule.format = exceptionpointLineFormat;
	highlightingRules.append(rule);

	//
	//
	QTextCharFormat xrefCommentFormat;
	xrefCommentFormat.setForeground(QColor("#00CEA5"));

	rule.pattern = QRegularExpression(" .xref[^\n]*");
	rule.format = xrefCommentFormat;
	highlightingRules.append(rule);

	//
	//
	QTextCharFormat stringCommentFormat;
	stringCommentFormat.setForeground(QColor("#ff9b77"));

	rule.pattern = QRegularExpression(" \"[^\n]*");
	rule.format = stringCommentFormat;
	highlightingRules.append(rule);

	QTextCharFormat lstringCommentFormat;
	lstringCommentFormat.setForeground(QColor("#ff9b77"));

	rule.pattern = QRegularExpression(" L\"[^\n]*");
	rule.format = lstringCommentFormat;
	highlightingRules.append(rule);
}

void SyntaxHighlighter::highlightBlock(const QString& text)
{
	foreach(const HighlightingRule &rule, highlightingRules) 
	{
		QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
		while (matchIterator.hasNext()) 
		{
			QRegularExpressionMatch match = matchIterator.next();
			setFormat(match.capturedStart(), match.capturedLength(), rule.format);
		}
	}

	if (m_highlightedString.isEmpty())
	{
		return;
	}

	QTextCharFormat fmt;
	QColor txt = QColor("#509CE4");
	fmt.setFont(QFont("Consolas", 9, QFont::Bold));
	fmt.setBackground(txt);
	//fmt.setForeground(QColor("red"));

	const int LEN = m_highlightedString.length();
	for (int index = text.indexOf(m_highlightedString); 0 <= index; index = text.indexOf(m_highlightedString, index + LEN))
	{
		setFormat(index, LEN, fmt);
	}
}

void SyntaxHighlighter::setHighlightedString(const QString& str)
{
	m_highlightedString = str;
	rehighlight();
}
