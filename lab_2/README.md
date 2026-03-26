# Задание №2 по предмету System Design

## Выполнил: Журавлев Кирилл М8О-102СВ-25

## Вариант 19: Система управления обучением

## 1. Проектирование REST API

### Таблица API Endpoints

| Метод | URL | Описание |
|---|---|---|
| POST | `/api/v1/auth/register` | Регистрация пользователя |
| POST | `/api/v1/auth/login` | Аутентификация, получение JWT-токена |
| GET | `/api/v1/users/by-login/{login}` | Поиск пользователя по логину |
| GET | `/api/v1/users/search` | Поиск пользователей по имени и/или фамилии |
| POST | `/api/v1/courses` | Создание курса |
| GET | `/api/v1/courses` | Получение списка курсов |
| POST | `/api/v1/courses/{course_id}/lessons` | Добавление урока в курс |
| GET | `/api/v1/courses/{course_id}/lessons` | Получение списка уроков курса |
| POST | `/api/v1/courses/{course_id}/enrollments` | Запись пользователя на курс |
| GET | `/api/v1/users/{user_id}/courses` | Получение списка курсов пользователя |
| POST | `/api/v1/lessons/{lesson_id}/progress` | Отметка урока как пройденного |
| GET | `/ping` | Health check сервиса |

### Таблица статус-кодов

| Код | Где используется | Описание |
|---|---|---|
| 200 | GET, POST action | Успешный запрос |
| 201 | POST create | Ресурс успешно создан |
| 400 | Любой endpoint | Некорректные данные запроса |
| 401 | Защищённые endpoint’ы | Требуется аутентификация / невалидный JWT |
| 403 | Защищённые endpoint’ы | Доступ запрещён по роли или правилам |
| 404 | GET, POST | Ресурс не найден |
| 409 | POST | Конфликт: дублирование логина, повторная запись на курс |
| 500 | Любой endpoint | Внутренняя ошибка сервера |

### Структура Request/Response для каждого endpoint

Все запросы и ответы сервиса передаются в формате **JSON**, кроме `GET /ping`, который используется как служебный health-check endpoint.

### Регистрация преподавателя
```bash
curl -X POST http://localhost:8080/api/v1/auth/register \
-H "Content-Type: application/json" \
-d '{"login":"teacher1","password":"pass123","first_name":"Иван","last_name":"Петров","role":"teacher"}'
```

Ответ (201 Created):
```json
{"id":3,"login":"teacher1","first_name":"Иван","last_name":"Петров","role":"teacher","status":"registered"}
```

Возможные статусы:

* 201 — пользователь создан
* 400 — некорректные данные
* 409 — логин уже существует
* 500 — внутренняя ошибка

### Аутентификация преподавателя
```bash
curl -X POST http://localhost:8080/api/v1/auth/login \
-H "Content-Type: application/json" \
-d '{"login":"teacher1","password":"pass123"}'
```
Ответ (200 OK):
```json
{"token":"eyJhbGciOiJIUzI1NiIsI...","token_type":"Bearer","login":"teacher1","role":"teacher"}
```
Возможные статусы:

* 200 — успешная аутентификация
* 400 — некорректные данные
* 401 — неверный логин или пароль
* 500 — ошибка сервера

### Получить преподавателя по логину
```bash
curl -X GET http://localhost:8080/api/v1/users/by-login/teacher1 \
-H "Authorization: Bearer <TOKEN>"
```
Ответ (200 OK):
```json
{"id":3,"login":"teacher1","first_name":"Иван","last_name":"Петров","role":"teacher"}
```
Возможные статусы:

* 200 — успешно
* 400 — некорректный login
* 401 — нет токена / невалидный JWT
* 404 — пользователь не найден
* 500 — ошибка сервера

### Создание курса
```bash
curl -X POST http://localhost:8080/api/v1/courses \
-H "Authorization: Bearer <TOKEN>" \
-H "Content-Type: application/json" \
-d '{"title":"C++ Basics","description":"Введение в C++","teacher_id":1}'
```
Ответ (201 Created):
```json
{"id":2,"title":"C++ Basics","description":"Введение в C++","teacher_id":1}
```
Возможные статусы:

* 201 — курс создан
* 400 — некорректные данные
* 401 — нет токена
* 403 — нет прав
* 404 — преподаватель не найден
* 500 — ошибка сервера

### Список курсов
```bash
curl -X GET http://localhost:8080/api/v1/courses \
-H "Authorization: Bearer <TOKEN>"
```
Ответ (200 OK):
```json
[{"id":2,"title":"C++ Basics","description":"Введение в C++","teacher_id":1},{"id":1,"title":"C++ Basics","description":"Intro course","teacher_id":1}]
```
Возможные статусы:

* 200 — успешно
* 401 — нет токена
* 500 — ошибка сервера

### Добавление урока
```bash
curl -X POST http://localhost:8080/api/v1/courses/1/lessons \
-H "Authorization: Bearer <TOKEN>" \
-H "Content-Type: application/json" \
-d '{"title":"Lesson 1","content":"Переменные и типы данных"}'
```
Ответ (201 Created):
```
{"id":2,"course_id":1,"title":"Lesson 1","content":"Переменные и типы данных"}
```
Возможные статусы:

* 201 — урок создан
* 400 — некорректные данные
* 401 — нет токена
* 403 — нет прав
* 404 — курс не найден
* 500 — ошибка сервера

Список уроков курса
```bash
curl -X GET http://localhost:8080/api/v1/courses/1/lessons \
-H "Authorization: Bearer <TOKEN>"
```
Ответ (200 OK):
```json
[{"id":1,"course_id":1,"title":"Lesson 1","content":"Variables and types"},{"id":2,"course_id":1,"title":"Lesson 1","content":"Переменные и типы данных"}]
```
Возможные статусы:

* 200 — успешно
* 401 — нет токена
* 404 — курс не найден
* 500 — ошибка сервера

### Запись на курс
```bash
curl -X POST http://localhost:8080/api/v1/courses/1/enrollments \
-H "Authorization: Bearer <TOKEN>" \
-H "Content-Type: application/json" \
-d '{"user_id":2}'
```
Ответ (200 OK):
```json
{"status":"enrolled","user_id":2,"course_id":1}
```
Возможные статусы:

* 200 — успешно
* 400 — некорректные данные
* 401 — нет токена
* 403 — нет прав
* 404 — пользователь или курс не найден
* 409 — уже записан
* 500 — ошибка сервера

### Курсы пользователя
```bash
curl -X GET http://localhost:8080/api/v1/users/2/courses \
-H "Authorization: Bearer <TOKEN>"
```
Ответ (200 OK):
```json
[{"id":1,"title":"C++ Basics","description":"Intro course","teacher_id":1}]
```
Возможные статусы:

* 200 — успешно
* 401 — нет токена
* 404 — пользователь не найден
* 500 — ошибка сервера

### Отметка урока как завершенного
```bash
curl -X POST http://localhost:8080/api/v1/lessons/1/progress \
-H "Authorization: Bearer <TOKEN>" \
-H "Content-Type: application/json" \
-d '{"user_id":2,"completed":true}'
```
Ответ (200 OK):
```json
{"status":"completed","user_id":2,"lesson_id":1,"completed":true}
```
Возможные статусы:

* 200 — успешно
* 400 — некорректные данные
* 401 — нет токена
* 403 — нет прав
* 404 — пользователь или урок не найден
* 500 — ошибка сервера

## 2. Реализация REST API сервиса

REST API сервис реализован на языке **C++** с использованием фреймворка **Yandex Userver**. Архитектура построена с разделением на обработчики (`views`), DTO, модели и слой хранения данных (`storage`).

Сервис реализует функциональность системы управления обучением (LMS): регистрацию и аутентификацию пользователей, работу с курсами и уроками, запись на курсы и отслеживание прогресса.


### Используемый стек

- Язык: **C++**
- Фреймворк: **Yandex Userver**
- Формат данных: **JSON**
- Аутентификация: **JWT**
- Хранилище: **in-memory**

### Хранилище данных

Используется **in-memory хранилище**, реализованное через:

- `std::unordered_map` — хранение сущностей  
- `std::mutex` + `std::lock_guard` — потокобезопасность  

Особенности:
- данные хранятся в памяти процесса  
- при перезапуске сервиса данные теряются  


### Использование DTO

Для передачи данных используются **DTO (Data Transfer Objects)**:

- `auth_dto` — логин/токен  
- `user_dto` — пользователь  
- `course_dto` — курс  
- `lesson_dto` — урок  
- `enrollment_dto` — запись  
- `progress_dto` — прогресс  

Преимущества:
- разделение API и внутренней логики  
- контроль структуры JSON  
- отсутствие лишних данных в ответе  


## 3. Реализация аутентификации

В сервисе реализована **JWT-аутентификация**.

Пользователь:
1. регистрируется  
2. логинится  
3. получает токен  
4. использует его в запросах 

## 4. Документирование API

Спецификация API находится в файле [lms_service/docs/openapi.yaml](lms_service/docs/openapi.yaml).

## 5. Тестирование

Для проверки работоспособности REST API были использованы встроенные тесты **Yandex Userver testsuite** на основе `pytest`.

Такой подход был выбран, потому что он позволяет:
- автоматически запускать тесты для основных endpoint’ов;
- проверять как успешные сценарии, так и обработку ошибок;
- воспроизводимо тестировать API без ручного ввода команд;
- интегрировать тестирование в общий пайплайн сборки проекта.

### Что проверяется

В тестах покрыты основные сценарии работы API:

#### Успешные сценарии
- регистрация пользователя;
- аутентификация пользователя;
- получение пользователя по логину;
- создание курса;
- получение списка курсов;
- добавление урока в курс;
- получение списка уроков курса;
- запись пользователя на курс;
- получение курсов пользователя;
- отметка урока как завершённого.

#### Ошибочные сценарии
- регистрация с неполными данными;
- логин с неверным паролем;
- запрос к защищённому endpoint без JWT;
- создание курса с пустыми данными;
- добавление урока с пустыми данными;
- запись на курс без `user_id`;
- повторная запись на курс;
- отметка прогресса с некорректным телом запроса.


### Запуск тестов

Для запуска всех тестов используется команда:

```bash
make test-debug
```
## Запуск сервиса в Docker

Файлы `Dockerfile` и `docker-compose.yaml` находятся в каталоге `lms_service`, поэтому перед запуском необходимо перейти в эту папку:

```bash
cd lms_service
docker compose up --build
```

После запуска API будет доступно по адресу: `http://localhost:8080`