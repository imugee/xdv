#ifndef __DEFINE_DBGHLPR_SYNTAX_HIGHLIGHTER__
#define __DEFINE_DBGHLPR_SYNTAX_HIGHLIGHTER__

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QTextDocument>

class SyntaxHighlighter : public QSyntaxHighlighter
{
public:
	SyntaxHighlighter(QTextDocument* parent);
	void highlightBlock(const QString& text);
	void setHighlightedString(const QString& str);

	void addHighlightBlock(char * expression, char * color, bool bold);

private:
	QString m_highlightedString;

private:
	struct HighlightingRule
	{
		QRegularExpression pattern;
		QTextCharFormat format;
	};
	QVector<HighlightingRule> highlightingRules;
};

#endif