#include "SyntaxHighlighter.h"

SyntaxHighlighter::SyntaxHighlighter(QTextDocument* parent) 
	: QSyntaxHighlighter(parent)
{
}

void SyntaxHighlighter::addHighlightBlock(char * expression, char * color, bool bold)
{
	HighlightingRule rule;
	QTextCharFormat format;
	format.setForeground(QColor(color));
	if (bold)
	{
		format.setFontWeight(QFont::Bold);
	}

	rule.pattern = QRegularExpression(expression);
	rule.format = format;
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
