<img width="1920" height="1046" alt="Kapraxis" src="https://github.com/user-attachments/assets/3dc706af-18d1-48d2-bb8c-e49f5d216a26" />

## Kapraxis

Desktop app para gerenciar questões com tags, imagens e busca. Importa notas do Google Keep.

<img width="1920" height="1080" alt="Interface" src="https://github.com/user-attachments/assets/8e4b32df-025e-4306-9670-1c4a22a8d9ba" />

<img width="1921" height="1052" alt="Editor" src="https://github.com/user-attachments/assets/b46e1e36-2b3a-4170-a53f-422e693620aa" />

<img width="1921" height="1045" alt="Filtros" src="https://github.com/user-attachments/assets/408075b3-0698-48aa-8dfa-706952eef9f9" />

### Stack

<img width="1919" height="534" alt="Stack" src="https://github.com/user-attachments/assets/f646cf56-1e82-43b6-9c9b-ea6a0c370f6f" />

- C++17
- Qt6
- SQLite
- CMake 3.16+

### Build

#### Dependências

Ubuntu/Debian:
```bash
sudo apt-get install qt6-base-dev cmake build-essential
```

macOS:
```bash
brew install qt cmake
```

Windows:
```bash
choco install cmake qt6-base
```


### Estrutura

```
src/
├── app/           AppWindow, main
├── ui/            QuestoesPage, BlocosPage, InicioPage, SettingsPage
├── repo/          QuestaoRepoSQLite, StudyStatsStore
└── utils/         ImageStore
```

### Atalhos

- Ctrl+N: Nova questão
- Ctrl+S: Salvar
- Ctrl+F: Buscar
- Delete: Deletar questão

### Download

[Releases](https://github.com/kaaylooon/kapraxis/releases)

---