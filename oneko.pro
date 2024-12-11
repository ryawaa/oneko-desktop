QT += core gui widgets
CONFIG += c++11
TARGET = oneko
TEMPLATE = app
SOURCES += oneko.cpp
HEADERS += oneko.h

# Generate moc file
QMAKE_EXTRA_COMPILERS += moc
moc.input = HEADERS
moc.output = moc_${QMAKE_FILE_BASE}.cpp
moc.commands = $$[QT_INSTALL_BINS]/moc ${QMAKE_FILE_NAME} -o ${QMAKE_FILE_OUT}
moc.variable_out = SOURCES
moc.name = MOC ${QMAKE_FILE_IN}
moc.CONFIG += target_predeps
