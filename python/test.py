#!/usr/bin/env python
from FakeVim import FakeVimHandler, FakeVimProxy, FAKEVIM_PYQT_VERSION
import sys

if FAKEVIM_PYQT_VERSION == 5:
    from PyQt5.QtCore import *
    from PyQt5.QtGui import *
    from PyQt5.QtWidgets import *
else:
    from PyQt4.QtCore import *
    from PyQt4.QtGui import *

def overrides(interface_class):
    """ Decorator for overriden methods. """
    def overrider(method):
        assert(method.__name__ in dir(interface_class))
        return method
    return overrider

class Proxy (FakeVimProxy):
    """ Used by FakeVim to modify or retrieve editor state. """
    def __init__(self, editor, handler):
        super(Proxy, self).__init__(handler)
        self.editor = editor

    @overrides(FakeVimProxy)
    def handleExCommand(self, cmd):
        if cmd.matches("q", "quit"):
            qApp.exit()
        else:
            return False
        return True

    @overrides(FakeVimProxy)
    def enableBlockSelection(self, cursor):
        self.editor.setTextCursor(cursor)
        self.editor.setBlockSelection(True)

    @overrides(FakeVimProxy)
    def disableBlockSelection(self):
        self.editor.setBlockSelection(False)

    @overrides(FakeVimProxy)
    def blockSelection(self):
        self.editor.setBlockSelection(True)
        return self.editor.textCursor()

    @overrides(FakeVimProxy)
    def hasBlockSelection(self):
        return self.editor.hasBlockSelection()


class Editor (QTextEdit):
    """ Editor widget driven by FakeVim. """
    def __init__(self):
        sup = super(Editor, self)
        sup.__init__()

        self.__context = QAbstractTextDocumentLayout.PaintContext()
        self.__lastCursorRect = QRect()
        self.__hasBlockSelection = False
        self.__selection = []
        self.__searchSelection = []

        self.viewport().installEventFilter(self)

        self.__handler = FakeVimHandler(self)
        self.__handler.installEventFilter()
        self.__handler.setupWidget()

        self.proxy = Proxy(self, self.__handler)

        self.selectionChanged.connect(self.__onSelectionChanged)
        self.cursorPositionChanged.connect(self.__onSelectionChanged)

    def setBlockSelection(self, enabled):
        self.__hasBlockSelection = enabled
        self.__selection.clear()
        self.__updateSelections()

    def hasBlockSelection(self):
        return self.__hasBlockSelection

    def eventFilter(self, viewport, paintEvent):
        """ Handle paint event from text editor. """
        if viewport != self.viewport() or paintEvent.type() != QEvent.Paint:
            return False

        tc = self.textCursor()

        self.__context.cursorPosition = -1
        self.__context.palette = self.palette()

        h = self.__horizontalOffset()
        v = self.__verticalOffset()
        self.__context.clip = QRectF( paintEvent.rect().translated(h, v) )

        painter = QPainter(viewport)

        painter.save()

        # Draw base and text.
        painter.translate(-h, -v)
        self.__paintDocument(painter)

        # Draw block selection.
        if self.hasBlockSelection():
            rect = self.cursorRect(tc)
            tc2 = QTextCursor(tc)
            tc2.setPosition(tc.anchor())
            rect = rect.united( self.cursorRect(tc2) )

            self.__context.palette.setColor( QPalette.Base,
                    self.__context.palette.color(QPalette.Highlight) )
            self.__context.palette.setColor( QPalette.Text,
                    self.__context.palette.color(QPalette.HighlightedText) )

            self.__context.clip = QRectF( rect.translated(h, v) )

            self.__paintDocument(painter)

        painter.restore()

        # Draw text cursor.
        rect = self.cursorRect()

        if self.overwriteMode() or self.hasBlockSelection():
            fm = self.fontMetrics()
            c = self.document().characterAt( tc.position() )
            rect.setWidth( fm.width(c) )
            if rect.width() == 0:
                rect.setWidth( fm.averageCharWidth() )
        else:
            rect.setWidth(2)
            rect.adjust(-1, 0, 0, 0)

        if self.hasBlockSelection():
            startColumn = self.__columnForPosition(tc.anchor())
            endColumn = self.__columnForPosition(tc.position())

            if startColumn < endColumn:
                rect.moveLeft(rect.left() - rect.width())

        painter.setCompositionMode(QPainter.CompositionMode_Difference)
        painter.fillRect(rect, Qt.white)

        if not self.hasBlockSelection() \
           and self.__lastCursorRect.width() != rect.width():
            viewport.update()

        self.__lastCursorRect = rect

        return True

    def __columnForPosition(self, position):
        return position - self.document().findBlock(position).position()

    def __paintDocument(self, painter):
        painter.setClipRect(self.__context.clip)
        painter.fillRect( self.__context.clip, self.__context.palette.base() )
        self.document().documentLayout().draw(painter, self.__context)

    def __horizontalOffset(self):
        hbar = self.horizontalScrollBar()

        if self.isRightToLeft():
            return hbar.maximum() - hbar.value()

        return hbar.value()

    def __verticalOffset(self):
        return self.verticalScrollBar().value()

    def __updateSelections(self):
        self.__context.selections = self.__searchSelection + self.__selection
        self.viewport().update()

    def __onSelectionChanged(self):
        self.__hasBlockSelection = False
        self.__selection.clear()

        selection = QAbstractTextDocumentLayout.Selection()
        selection.cursor = self.textCursor()

        if selection.cursor.hasSelection():
            pal = self.palette()
            selection.format.setBackground( pal.color(QPalette.Highlight) )
            selection.format.setForeground( pal.color(QPalette.HighlightedText) )
            self.__selection.append(selection)

        self.__updateSelections()

if __name__ == "__main__":
    app = QApplication(sys.argv)

    editor = Editor()
    font = editor.font()
    font.setFamily("Monospace")
    editor.setFont(font)
    editor.move(0, 0)
    editor.resize(600, 600)
    editor.setText("""
        ABCDEFGHIJKLMNOPQRSTUVWXZY
        ABCDEFGHIJKLMNOPQRSTUVWXZY
        ABCDEFGHIJKLMNOPQRSTUVWXZY
        ABCDEFGHIJKLMNOPQRSTUVWXZY
        ABCDEFGHIJKLMNOPQRSTUVWXZY
        ABCDEFGHIJKLMNOPQRSTUVWXZY
        ABCDEFGHIJKLMNOPQRSTUVWXZY
        ABCDEFGHIJKLMNOPQRSTUVWXZY
        ABCDEFGHIJKLMNOPQRSTUVWXZY
        ABCDEFGHIJKLMNOPQRSTUVWXZY
    """)
    editor.show()

    sys.exit(app.exec_())

