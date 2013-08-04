#!/usr/bin/python2
from PyQt4.QtCore import *
from PyQt4.QtGui import *
import sys
import FakeVim

class Editor (QTextEdit):
    def __init__(self):
        sup = super(Editor, self)
        sup.__init__()
        sup.setCursorWidth(0);

    def paintEvent(self, e):
        sup = super(Editor, self)
        sup.paintEvent(e)

        # Draw text cursor.
        rect = sup.cursorRect()
        if e.rect().contains(rect):
            painter = QPainter(sup.viewport())

            if sup.overwriteMode():
                fm = QFontMetrics(sup.font())
                rect.setWidth(fm.width('m'))
                painter.setPen(Qt.NoPen)
                painter.setBrush(sup.palette().color(QPalette.Base))
                painter.setCompositionMode(QPainter.CompositionMode_Difference)
            else:
                rect.setWidth(sup.cursorWidth());
                painter.setPen(sup.palette().color(QPalette.Text));

            painter.drawRect(rect);

if __name__ == "__main__":
    app = QApplication(sys.argv)

    editor = Editor()
    font = editor.font()
    font.setFamily("Monospace")
    editor.setFont(font)
    editor.show()

    handler = FakeVim.FakeVim.Internal.FakeVimHandler(editor)
    handler.installEventFilter();
    handler.setupWidget();

    sys.exit(app.exec_())

