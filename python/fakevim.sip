%Module FakeVim

%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip

namespace FakeVim {

%TypeHeaderCode
#include <fakevim/fakevimhandler.h>
%End

namespace Internal {

%TypeHeaderCode
#include <fakevim/fakevimhandler.h>
%End

enum RangeMode
{
    // Reordering first three enum items here will break
    // compatibility with clipboard format stored by Vim.
    RangeCharMode,         // v
    RangeLineMode,         // V
    RangeBlockMode,        // Ctrl-v
    RangeLineModeExclusive,
    RangeBlockAndTailMode // Ctrl-v for D and X
};

/*
struct Range
{

%TypeHeaderCode
#include <fakevim/fakevimhandler.h>
%End

    Range();
    Range(int b, int e, RangeMode m = RangeCharMode);
    QString toString() const;

    int beginPos;
    int endPos;
    RangeMode rangemode;
};

struct ExCommand
{

%TypeHeaderCode
#include <fakevim/fakevimhandler.h>
%End

    ExCommand();
    ExCommand(const QString &cmd, const QString &args = QString(),
        const Range &range = Range());

    bool matches(const QString &min, const QString &full) const;

    QString cmd;
    bool hasBang;
    QString args;
    Range range;
    int count;
};

// message levels sorted by severity
enum MessageLevel
{
    MessageMode,    // show current mode (format "-- %1 --")
    MessageCommand, // show last Ex command or search
    MessageInfo,    // result of a command
    MessageWarning, // warning
    MessageError,   // error
    MessageShowCmd  // partial command
};
*/

class FakeVimHandler /NoDefaultCtors/ {

%TypeHeaderCode
#include <fakevim/fakevimhandler.h>
%End

public:
    explicit FakeVimHandler(QWidget *widget, QObject *parent /TransferThis/ = 0);
    ~FakeVimHandler();

    QWidget *widget();

    // call before widget is deleted
    void disconnectFromEditor();

public slots:
    void setCurrentFileName(const QString &fileName);
    QString currentFileName() const;

    //void showMessage(MessageLevel level, const QString &msg);

    // This executes an "ex" style command taking context
    // information from the current widget.
    void handleCommand(const QString &cmd);
    void handleReplay(const QString &keys);
    void handleInput(const QString &keys);

    void installEventFilter();

    // Convenience
    void setupWidget();
    void restoreWidget(int tabSize);

    // Test only
    int physicalIndentation(const QString &line) const;
    int logicalIndentation(const QString &line) const;
    QString tabExpand(int n) const;

    void miniBufferTextEdited(const QString &text, int cursorPos, int anchorPos);

    // Set text cursor position. Keeps anchor if in visual mode.
    void setTextCursorPosition(int position);

    QTextCursor textCursor() const;
    void setTextCursor(const QTextCursor &cursor);

    bool jumpToLocalMark(QChar mark, bool backTickMode);

signals:
    void commandBufferChanged(const QString &msg, int cursorPos,
        int anchorPos, int messageLevel, QObject *eventFilter);
    void statusDataChanged(const QString &msg);
    void extraInformationChanged(const QString &msg);
    void selectionChanged(const QList<QTextEdit::ExtraSelection> &selection);
    void highlightMatches(const QString &needle);
    void writeAllRequested(QString *error);
    /*void moveToMatchingParenthesis(bool *moved, bool *forward, QTextCursor *cursor);*/
    /*void checkForElectricCharacter(bool *result, QChar c);*/
    void indentRegion(int beginLine, int endLine, QChar typedChar);
    void completionRequested();
    void simpleCompletionRequested(const QString &needle, bool forward);
    void windowCommandRequested(const QString &key, int count);
    void findRequested(bool reverse);
    void findNextRequested(bool reverse);
    /*void handleExCommandRequested(bool *handled, const ExCommand &cmd);*/
    void requestSetBlockSelection(bool on);
    /*void requestHasBlockSelection(bool *on);*/
    void foldToggle(int depth);
    void foldAll(bool fold);
    void fold(int depth, bool fold);
    void foldGoTo(int count, bool current);
    void jumpToGlobalMark(QChar mark, bool backTickMode, const QString &fileName);
};

}; // namespace FakeVim
}; // namespace Internal

