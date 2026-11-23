# BMSTU_IU7_2026_Algorithm_analisys

В анализе алгоритмов предполагается построение диаграмм алгоритмов,
написание реализации на языке приграммирования и написание отчета.

> Большая просьба (в противном случае возникнут сложности со сдачей) сохранять структуру репозиториев:

```bash
├── test/   # Должна быть пустой. Не предназначена для файлов студента. Не имеет никакого отношения к собственным тестам.
├── code/
└── report
    ├── [директории с tex-файлами, картинками и пр.]
    └── report.pdf (не больше 15МБ)
```

Вот вариант организации репозитория не противоречащий общим требованиям:
```bash
.
├── algorithm_diagram_playground    # Он же algirithm_diagram_factory с менее подозрительным названием. Содержимое добавь в .gitignore.
├── code    # Формальных требований к содержимому директории нет.
│   ├── Makefile
│   ├── main.cpp
│   ├── test.cpp
│   └── ...
├── report
│   ├── diagrams    # Сюда положи .graphml файлы
│   │   ├── linear_control_flow_graph.graphml
│   │   ├── ...
│   │   └── vector  # Сюда можно положить генерируемые svg файлы.
│   │       ├── linear_control_flow_graph.svg
│   │       └── ...
│   └── tex
│       ├── bmstu-appendix.sty  # Позаимствованные шаблоны титульников и прочих стандартов из репозитория bmstu (о нем ниже).
│       ├── bmstu-biblio.sty
│       ├── bmstu-defabbr.sty
│       ├── bmstu-essay.sty
│       ├── bmstu-figure.sty
│       ├── bmstu-listing.sty
│       ├── bmstu-logo.pdf
│       ├── bmstu-title.sty
│       ├── bmstu-toc.sty
│       ├── bmstu.cls
│       ├── build
│       │   ├── report.pdf  # НАДО выгружать в репозиторий.
│       │   └── ...
│       ├── manifest.txt    # Этот файл тоже позаимствован. 
│       ├── references.bib
│       ├── report.pdf -> build/report.pdf
│       ├── report.tex
└── test/   # Должна быть пустой. Не предназначена для файлов студента. Не имеет никакого отношения к собственным тестам.
```

- `aa-report-gost-skill/` - копия [репозитория](https://github.com/bmstu-software-engineering/aa-report-gost-skill) со скиллом для написания отчетов по ГОСТу. [Гайд](https://t.me/c/2216719917/3/238186) от Никиты.
- `algirithm_diagram_factory/` - папка с промптом и конвертером для быстрого создания
  схемы алгоритма по человеческому описанию и условию задачи. [Гайд](https://t.me/c/2216719917/1/227877) и файлы от Руслана.
- `bmstu/` - репозиторий пакета Latex с шаблонами документов для МГТУ. Уже включен в
  стандартный Latex, но там устаревшая шапка титульника (бюджетное а не автономное). Рекомендуется взять копию [репозитория](https://github.com/Orianti/bmstu-latex-class) для локального исправления и приоритетной подстановки.
- `diagrams/` - папка для хранения исходников схем алгоритмов и векторных изображений.
- `reports/` - директория с исходниками отчета по каждой лабораторной работе.
- `code/` - папка для реализаций алгоритмов на языке программирования.

Шаблон `.gitignore`:
```
*.aux
*.fdb_latexmk
*.fls
*.glg
*.glo
*.gls
*.idx
*.ilg
*.ind
*.ist
*.log
*.out
*.toc
*.nav
*.snm
*.vrb
*.bbl
*.bcf
*.blg
*.run.xml

*.synctex
*.synctex(busy)
*.synctex.gz
*.synctex.gz(busy)

build/

*.exe
*.out
*.o
*.build/
bin/
obj/

*.pdf
*.csv

!tasks/*
!bmstu-logo.pdf

.venv/
algorithm_diagram_playground/
```