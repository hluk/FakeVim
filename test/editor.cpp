/*
    Copyright (c) 2017, Lukas Holecek <hluk@email.cz>

    This file is part of CopyQ.

    CopyQ is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CopyQ is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CopyQ.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "fakevimactions.h"
#include "fakevimhandler.h"

#include <QApplication>
#include <QFontMetrics>
#include <QMainWindow>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QStatusBar>
#include <QTextBlock>
#include <QTextEdit>
#include <QTextStream>
#include <QTemporaryFile>

#define EDITOR(editor, call) \
    if (QPlainTextEdit *ed = qobject_cast<QPlainTextEdit *>(editor)) { \
        (ed->call); \
    } else if (QTextEdit *ed = qobject_cast<QTextEdit *>(editor)) { \
        (ed->call); \
    }

using namespace FakeVim::Internal;

typedef QLatin1String _;

/**
 * Simple editor widget.
 * @tparam TextEdit QTextEdit or QPlainTextEdit as base class
 */
template <typename TextEdit>
class Editor : public TextEdit
{
public:
    Editor(QWidget *parent = 0) : TextEdit(parent)
    {
        TextEdit::setCursorWidth(0);
    }

    void paintEvent(QPaintEvent *e)
    {
        TextEdit::paintEvent(e);

        if ( !m_cursorRect.isNull() && e->rect().intersects(m_cursorRect) ) {
            QRect rect = m_cursorRect;
            m_cursorRect = QRect();
            TextEdit::viewport()->update(rect);
        }

        // Draw text cursor.
        QRect rect = TextEdit::cursorRect();
        if ( e->rect().intersects(rect) ) {
            QPainter painter(TextEdit::viewport());

            if ( TextEdit::overwriteMode() ) {
                QFontMetrics fm(TextEdit::font());
                const int position = TextEdit::textCursor().position();
                const QChar c = TextEdit::document()->characterAt(position);
                rect.setWidth(fm.width(c));
                painter.setPen(Qt::NoPen);
                painter.setBrush(TextEdit::palette().color(QPalette::Base));
                painter.setCompositionMode(QPainter::CompositionMode_Difference);
            } else {
                rect.setWidth(TextEdit::cursorWidth());
                painter.setPen(TextEdit::palette().color(QPalette::Text));
            }

            painter.drawRect(rect);
            m_cursorRect = rect;
        }
    }

private:
    QRect m_cursorRect;
};

class Proxy : public QObject
{
    Q_OBJECT

public:
    Proxy(QWidget *widget, QMainWindow *mw, QObject *parent = 0)
      : QObject(parent), m_widget(widget), m_mainWindow(mw)
    {
    }

    void openFile(const QString &fileName)
    {
        emit handleInput(QString(_(":r %1<CR>")).arg(fileName));
        m_fileName = fileName;
    }

signals:
    void handleInput(const QString &keys);

public slots:
    void changeStatusData(const QString &info)
    {
        m_statusData = info;
        updateStatusBar();
    }

    void highlightMatches(const QString &pattern)
    {
        QTextEdit *ed = qobject_cast<QTextEdit *>(m_widget);
        if (!ed)
            return;

        QTextCursor cur = ed->textCursor();

        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(Qt::yellow);
        selection.format.setForeground(Qt::black);

        // Highlight matches.
        QTextDocument *doc = ed->document();
        QRegExp re(pattern);
        cur = doc->find(re);

        m_searchSelection.clear();

        int a = cur.position();
        while ( !cur.isNull() ) {
            if ( cur.hasSelection() ) {
                selection.cursor = cur;
                m_searchSelection.append(selection);
            } else {
                cur.movePosition(QTextCursor::NextCharacter);
            }
            cur = doc->find(re, cur);
            int b = cur.position();
            if (a == b) {
                cur.movePosition(QTextCursor::NextCharacter);
                cur = doc->find(re, cur);
                b = cur.position();
                if (a == b) break;
            }
            a = b;
        }

        updateExtraSelections();
    }

    void changeStatusMessage(const QString &contents, int cursorPos)
    {
        m_statusMessage = cursorPos == -1 ? contents
            : contents.left(cursorPos) + QChar(10073) + contents.mid(cursorPos);
        updateStatusBar();
    }

    void changeExtraInformation(const QString &info)
    {
        QMessageBox::information(m_widget, tr("Information"), info);
    }

    void updateStatusBar()
    {
        int slack = 80 - m_statusMessage.size() - m_statusData.size();
        QString msg = m_statusMessage + QString(slack, QLatin1Char(' ')) + m_statusData;
        m_mainWindow->statusBar()->showMessage(msg);
    }

    void handleExCommand(bool *handled, const ExCommand &cmd)
    {
        if ( wantSaveAndQuit(cmd) ) {
            // :wq
            if (save())
                cancel();
        } else if ( wantSave(cmd) ) {
            save(); // :w
        } else if ( wantQuit(cmd) ) {
            if (cmd.hasBang)
                invalidate(); // :q!
            else
                cancel(); // :q
        } else {
            *handled = false;
            return;
        }

        *handled = true;
    }

    void requestSetBlockSelection(const QTextCursor &tc)
    {
        QTextEdit *ed = qobject_cast<QTextEdit *>(m_widget);
        if (!ed)
            return;

        QPalette pal = ed->parentWidget() != NULL ? ed->parentWidget()->palette()
                                                  : QApplication::palette();

        m_blockSelection.clear();
        m_clearSelection.clear();

        QTextCursor cur = tc;

        QTextEdit::ExtraSelection selection;
        selection.format.setBackground( pal.color(QPalette::Base) );
        selection.format.setForeground( pal.color(QPalette::Text) );
        selection.cursor = cur;
        m_clearSelection.append(selection);

        selection.format.setBackground( pal.color(QPalette::Highlight) );
        selection.format.setForeground( pal.color(QPalette::HighlightedText) );

        int from = cur.positionInBlock();
        int to = cur.anchor() - cur.document()->findBlock(cur.anchor()).position();
        const int min = qMin(cur.position(), cur.anchor());
        const int max = qMax(cur.position(), cur.anchor());
        for ( QTextBlock block = cur.document()->findBlock(min);
              block.isValid() && block.position() < max; block = block.next() ) {
            cur.setPosition( block.position() + qMin(from, block.length()) );
            cur.setPosition( block.position() + qMin(to, block.length()), QTextCursor::KeepAnchor );
            selection.cursor = cur;
            m_blockSelection.append(selection);
        }

        disconnect( ed, &QTextEdit::selectionChanged,
                    this, &Proxy::updateBlockSelection );
        ed->setTextCursor(tc);
        connect( ed, &QTextEdit::selectionChanged,
                 this, &Proxy::updateBlockSelection );

        QPalette pal2 = ed->palette();
        pal2.setColor(QPalette::Highlight, Qt::transparent);
        pal2.setColor(QPalette::HighlightedText, Qt::transparent);
        ed->setPalette(pal2);

        updateExtraSelections();
    }

    void requestDisableBlockSelection()
    {
        QTextEdit *ed = qobject_cast<QTextEdit *>(m_widget);
        if (!ed)
            return;

        QPalette pal = ed->parentWidget() != NULL ? ed->parentWidget()->palette()
                                                  : QApplication::palette();

        m_blockSelection.clear();
        m_clearSelection.clear();

        ed->setPalette(pal);

        disconnect( ed, &QTextEdit::selectionChanged,
                    this, &Proxy::updateBlockSelection );

        updateExtraSelections();
    }

    void updateBlockSelection()
    {
        QTextEdit *ed = qobject_cast<QTextEdit *>(m_widget);
        if (!ed)
            return;

        requestSetBlockSelection(ed->textCursor());
    }

    void requestHasBlockSelection(bool *on)
    {
        *on = !m_blockSelection.isEmpty();
    }

    void indentRegion(int beginBlock, int endBlock, QChar typedChar)
    {
        QTextEdit *ed = qobject_cast<QTextEdit *>(m_widget);
        if (!ed)
            return;

        const auto indentSize = theFakeVimSetting(ConfigShiftWidth)->value().toInt();

        QTextDocument *doc = ed->document();
        QTextBlock startBlock = doc->findBlockByNumber(beginBlock);

        // Record line lenghts for mark adjustments
        QVector<int> lineLengths(endBlock - beginBlock + 1);
        QTextBlock block = startBlock;

        for (int i = beginBlock; i <= endBlock; ++i) {
            const auto line = block.text();
            lineLengths[i - beginBlock] = line.length();
            if (typedChar.unicode() == 0 && line.simplified().isEmpty()) {
                // clear empty lines
                QTextCursor cursor(block);
                while (!cursor.atBlockEnd())
                    cursor.deleteChar();
            } else {
                const auto previousBlock = block.previous();
                const auto previousLine = previousBlock.isValid() ? previousBlock.text() : QString();

                int indent = firstNonSpace(previousLine);
                if (typedChar == '}')
                    indent = std::max(0, indent - indentSize);
                else if ( previousLine.endsWith("{") )
                    indent += indentSize;
                const auto indentString = QString(" ").repeated(indent);

                QTextCursor cursor(block);
                cursor.beginEditBlock();
                cursor.movePosition(QTextCursor::StartOfBlock);
                cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, firstNonSpace(line));
                cursor.removeSelectedText();
                cursor.insertText(indentString);
                cursor.endEditBlock();
            }
            block = block.next();
        }
    }

    void checkForElectricCharacter(bool *result, QChar c)
    {
        *result = c == '{' || c == '}';
    }

private:
    static int firstNonSpace(const QString &text)
    {
        int indent = 0;
        while ( indent < text.length() && text.at(indent) == ' ' )
            ++indent;
        return indent;
    }

    void updateExtraSelections()
    {
        QTextEdit *ed = qobject_cast<QTextEdit *>(m_widget);
        if (ed)
            ed->setExtraSelections(m_clearSelection + m_searchSelection + m_blockSelection);
    }

    bool wantSaveAndQuit(const ExCommand &cmd)
    {
        return cmd.cmd == "wq";
    }

    bool wantSave(const ExCommand &cmd)
    {
        return cmd.matches("w", "write") || cmd.matches("wa", "wall");
    }

    bool wantQuit(const ExCommand &cmd)
    {
        return cmd.matches("q", "quit") || cmd.matches("qa", "qall");
    }

    bool save()
    {
        if (!hasChanges())
            return true;

        QTemporaryFile tmpFile;
        if (!tmpFile.open()) {
            QMessageBox::critical(m_widget, tr("FakeVim Error"),
                                  tr("Cannot create temporary file: %1").arg(tmpFile.errorString()));
            return false;
        }

        QTextStream ts(&tmpFile);
        ts << content();
        ts.flush();

        QFile::remove(m_fileName);
        if (!QFile::copy(tmpFile.fileName(), m_fileName)) {
            QMessageBox::critical(m_widget, tr("FakeVim Error"),
                                  tr("Cannot write to file \"%1\"").arg(m_fileName));
            return false;
        }

        return true;
    }

    void cancel()
    {
        if (hasChanges()) {
            QMessageBox::critical(m_widget, tr("FakeVim Warning"),
                                  tr("File \"%1\" was changed").arg(m_fileName));
        } else {
            invalidate();
        }
    }

    void invalidate()
    {
        QApplication::quit();
    }

    bool hasChanges()
    {
        if (m_fileName.isEmpty() && content().isEmpty())
            return false;

        QFile f(m_fileName);
        if (!f.open(QIODevice::ReadOnly))
            return true;

        QTextStream ts(&f);
        return content() != ts.readAll();
    }

    QTextDocument *document() const
    {
        QTextDocument *doc = NULL;
        if (QPlainTextEdit *ed = qobject_cast<QPlainTextEdit *>(m_widget))
            doc = ed->document();
        else if (QTextEdit *ed = qobject_cast<QTextEdit *>(m_widget))
            doc = ed->document();
        return doc;
    }

    QString content() const
    {
        return document()->toPlainText();
    }

    QWidget *m_widget;
    QMainWindow *m_mainWindow;
    QString m_statusMessage;
    QString m_statusData;
    QString m_fileName;

    QList<QTextEdit::ExtraSelection> m_searchSelection;
    QList<QTextEdit::ExtraSelection> m_clearSelection;
    QList<QTextEdit::ExtraSelection> m_blockSelection;
};

QWidget *createEditorWidget(bool usePlainTextEdit)
{
    QWidget *editor = 0;
    if (usePlainTextEdit) {
        Editor<QPlainTextEdit> *w = new Editor<QPlainTextEdit>;
        w->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        editor = w;
    } else {
        Editor<QTextEdit> *w = new Editor<QTextEdit>;
        w->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        editor = w;
    }
    editor->setObjectName(_("Editor"));
    editor->setFocus();

    return editor;
}

void initHandler(FakeVimHandler *handler)
{
    handler->handleCommand(_("set nopasskeys"));
    handler->handleCommand(_("set nopasscontrolkey"));

    // Set some Vim options.
    handler->handleCommand(_("set expandtab"));
    handler->handleCommand(_("set shiftwidth=8"));
    handler->handleCommand(_("set tabstop=16"));
    handler->handleCommand(_("set autoindent"));
    handler->handleCommand(_("set smartindent"));

    handler->installEventFilter();
    handler->setupWidget();
}

void initMainWindow(QMainWindow *mainWindow, QWidget *centralWidget, const QString &title)
{
    mainWindow->setWindowTitle(QString(_("FakeVim (%1)")).arg(title));
    mainWindow->setCentralWidget(centralWidget);
    mainWindow->resize(600, 650);
    mainWindow->move(0, 0);
    mainWindow->show();

    // Set monospace font for editor and status bar.
    QFont font = QApplication::font();
    font.setFamily(_("Monospace"));
    centralWidget->setFont(font);
    mainWindow->statusBar()->setFont(font);
}

void clearUndoRedo(QWidget *editor)
{
    EDITOR(editor, setUndoRedoEnabled(false));
    EDITOR(editor, setUndoRedoEnabled(true));
}

void connectSignals(
        FakeVimHandler *handler, QMainWindow *mainWindow, QWidget *editor,
        const QString &fileToEdit)
{
    auto proxy = new Proxy(editor, mainWindow, handler);

    QObject::connect(handler, &FakeVimHandler::commandBufferChanged,
        proxy, &Proxy::changeStatusMessage);
    QObject::connect(handler, &FakeVimHandler::extraInformationChanged,
        proxy, &Proxy::changeExtraInformation);
    QObject::connect(handler, &FakeVimHandler::statusDataChanged,
        proxy, &Proxy::changeStatusData);
    QObject::connect(handler, &FakeVimHandler::highlightMatches,
        proxy, &Proxy::highlightMatches);
    QObject::connect(handler, &FakeVimHandler::handleExCommandRequested,
        proxy, &Proxy::handleExCommand);
    QObject::connect(handler, &FakeVimHandler::requestSetBlockSelection,
        proxy, &Proxy::requestSetBlockSelection);
    QObject::connect(handler, &FakeVimHandler::requestDisableBlockSelection,
        proxy, &Proxy::requestDisableBlockSelection);
    QObject::connect(handler, &FakeVimHandler::requestHasBlockSelection,
        proxy, &Proxy::requestHasBlockSelection);

    QObject::connect(handler, &FakeVimHandler::indentRegion,
        proxy, &Proxy::indentRegion);
    QObject::connect(handler, &FakeVimHandler::checkForElectricCharacter,
        proxy, &Proxy::checkForElectricCharacter);

    QObject::connect(proxy, &Proxy::handleInput,
        handler, [handler] (const QString &text) { handler->handleInput(text); });

    if (!fileToEdit.isEmpty())
        proxy->openFile(fileToEdit);
}

#include "editor.moc"
