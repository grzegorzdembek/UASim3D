# UASim3D 

## 🛠️ Wymagania
* **System operacyjny:** Windows
* **Środowisko:** Visual Studio z zainstalowanym pakietem "Desktop development with C++" (Programowanie aplikacji klasycznych w języku C++).
* **Git:** Zainstalowany klient Git.

---

## 🚀 Pierwsze uruchomienie (Krok po kroku)

1. **Pobranie repozytorium (Clone):**
   Otwórz terminal w folderze, w którym chcesz trzymać projekt i wpisz komendę:
   ```bash
   git clone <TUTAJ_WKLEJ_LINK_DO_REPOZYTORIUM>

2. **Otwarcie projektu:**
   Wejdź do pobranego folderu i kliknij dwukrotnie plik `UASim3D.sln`. Projekt otworzy się w Visual Studio.

3. **Konfiguracja kompilacji:**
   Na górnym pasku narzędzi w Visual Studio upewnij się, że masz wybrane następujące opcje:
   * Tryb kompilacji: **`Release`**
   * Platforma: **`x64`**

4. **Kompilacja i uruchomienie:**
   * Wciśnij skrót **Ctrl + Shift + B** (lub wybierz z górnego menu `Kompiluj -> Kompiluj rozwiązanie`), aby zbudować projekt. Jeśli wszystko zostało pobrane poprawnie, powinieneś zobaczyć komunikat o sukcesie (Zakończono pomyślnie: 1).
   * Wciśnij **F5** (lub zielony przycisk "Lokalny debuger systemu Windows"), aby uruchomić aplikację.

---

## 🌿 Jak pracować z Git 

Aby uniknąć konfliktów w kodzie, **nie pracujemy bezpośrednio na gałęzi `main`**. Każdą nową funkcję lub poprawkę dodajemy na osobnej, dedykowanej gałęzi.

### 1. Rozpoczęcie pracy (Aktualizacja)
Zanim zaczniesz pisać kod, upewnij się, że masz najnowszą wersję projektu:
```bash
git checkout main
git pull
```

### 2. Utworzenie nowej gałęzi (Branch)
Stwórz nową gałąź i nadaj jej nazwę nawiązującą do tego, co będziesz robić:
```bash
git checkout -b nazwa-twojego-zadania
```

### 3. Zapisywanie zmian (Commit)
Gdy napiszesz kod i chcesz go zapisać w historii:
```bash
# Dodanie wszystkich zmienionych plików do poczekalni:
git add .

# Zapisanie zmian z krótkim opisem:
git commit -m "Krótki opis, co zostało zrobione"
```

### 4. Wysłanie zmian na serwer (Push)
Przy pierwszym wysyłaniu nowej gałęzi na GitHub użyj poniższej komendy (dzięki fladze `-u` powiążesz gałąź lokalną z tą na serwerze):
```bash
git push -u origin nazwa-twojego-zadania
```
*Przy kolejnych zmianach na tej samej gałęzi wystarczy samo `git push`.*

### 5. Zakończenie pracy (Pull Request)
Po wysłaniu zmian, wejdź na stronę repozytorium na GitHubie. Zobaczysz tam przycisk **"Compare & pull request"**. Kliknij