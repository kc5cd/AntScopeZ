<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="es_ES">
<!--
  App-authored patch for known qtbase_es.qm gaps; see
  MainWindow::m_qtBaseOverrideTranslator's declaration comment
  (mainwindow.h) and loadLanguage() (mainwindow_settings.cpp) for how this
  is loaded. Not lupdate-managed: these source strings don't come from
  any tr() call in this app's own code, they're Qt's own internal
  qtbase.ts entries whose shipped translation is unreachable at runtime
  because the source text here doesn't match Qt's real lookup key. Keep
  entries here to the minimum needed to fix the specific mismatch found;
  don't run this file through lupdate against AntScopeZ's sources, it
  would just get marked obsolete and stripped.
-->
<context>
    <name>QFileDialog</name>
    <message>
        <!--
          Found 2026-08-24: qtbase_es.ts (and every other qtbase_*.ts Qt
          ships) only has a translation for "Files of type:" (no
          ampersand), but the actual compiled QFileDialog widget requests
          "Files of &amp;type:" (mnemonic accelerator on the "t" in "type"),
          confirmed via `strings` against libQt6Widgets.so.6. Exact-string
          lookup miss, so every language's real translation work for this
          label is silently orphaned; Qt shows the raw English source
          instead regardless of the active language. Re-uses qtbase_es.qm's
          own existing (correct) wording for "Ficheros de tipo:", just adds
          the matching mnemonic on the same letter as the English source.
        -->
        <source>Files of &amp;type:</source>
        <translation>Ficheros de &amp;tipo:</translation>
    </message>
</context>
</TS>
