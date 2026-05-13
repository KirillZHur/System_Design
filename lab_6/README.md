# Задание №6 по предмету System Design

## Выполнил: Журавлев Кирилл М8О-102СВ-25

## Вариант 19: Система управления обучением

## 1. Анализ событий в системе

В рамках лабораторной работы для LMS выполнен анализ событий, команд и сервисов, которые должны реагировать на изменения состояния системы.

Система управления обучением содержит следующие основные сущности:

- пользователь;
- курс;
- урок;
- запись пользователя на курс;
- прогресс прохождения уроков.

Для применения паттерна CQRS выбрана сущность `Course`, так как создание курса является изменением состояния системы и может быть отделено от операций чтения.

### Команды

Команда — это намерение пользователя или сервиса изменить состояние системы.

| Команда | Описание | Инициатор | Результирующее событие |
|---|---|---|---|
| `CreateUser` | Создание нового пользователя | Клиентское приложение | `UserCreated` |
| `CreateCourse` | Создание нового курса | Преподаватель | `CourseCreateRequested`, затем `CourseCreated` |
| `AddLessonToCourse` | Добавление урока в курс | Преподаватель | `LessonAddedToCourse` |
| `EnrollUserToCourse` | Запись пользователя на курс | Студент | `UserEnrolledToCourse` |
| `MarkLessonCompleted` | Отметка урока как пройденного | Студент | `LessonCompleted` |

### События

Событие — это факт, который уже произошёл в системе.

| Событие | Описание | Производитель | Потребители |
|---|---|---|---|
| `UserCreated` | Создан новый пользователь | User Service | Auth Service, Notification Service |
| `CourseCreateRequested` | Получена команда на создание курса | Course Command Service | Course Consumer Service |
| `CourseCreated` | Курс сохранён в read-модели | Course Consumer Service | Course Service, Analytics Service, Notification Service |
| `LessonAddedToCourse` | В курс добавлен урок | Lesson Service | Course Service, Progress Service |
| `UserEnrolledToCourse` | Пользователь записан на курс | Enrollment Service | Course Service, Progress Service, Notification Service |
| `LessonCompleted` | Пользователь завершил урок | Progress Service | Course Service, Analytics Service |

### Сервисы, уведомляемые о событиях

Взаимодействие между компонентами системы выполняется через события Kafka. Сервисы подписываются только на те события, которые необходимы для их работы.

#### UserCreated

| Сервис | Назначение |
|---|---|
| Auth Service | Создание учётных данных пользователя |
| User Service | Обновление read-модели пользователей |

#### CourseCreateRequested

| Сервис | Назначение |
|---|---|
| Course Consumer Service | Сохранение курса в PostgreSQL и MongoDB |

#### CourseCreated

| Сервис | Назначение |
|---|---|
| Course Service | Обновление read-модели курсов |
| Enrollment Service | Подготовка возможности записи пользователей |
| Lesson Service | Работа с уроками курса |

#### LessonAddedToCourse

| Сервис | Назначение |
|---|---|
| Course Service | Обновление агрегата курса |
| Progress Service | Подготовка прогресса прохождения |

#### UserEnrolledToCourse

| Сервис | Назначение |
|---|---|
| Progress Service | Создание начального прогресса пользователя |
| Course Service | Обновление информации о количестве студентов |

#### LessonCompleted

| Сервис | Назначение |
|---|---|
| Progress Service | Пересчёт прогресса пользователя |
| Course Service | Обновление информации о прохождении курса |

### Событие, реализуемое в ЛР №6

В рамках реализации лабораторной работы подробно прорабатывается команда `CreateCourse`.

Для неё используется следующий поток:

1. Клиент отправляет запрос на создание курса.
2. `Course Command Service` принимает команду `CreateCourse`.
3. Вместо прямой записи курса в базу данных сервис публикует сообщение в Kafka topic.
4. Дополнительно команда может быть сохранена в Redis как временная command-модель.
5. `Course Consumer Service` считывает сообщение из Kafka.
6. Consumer сохраняет курс в основную базу данных.
7. После сохранения формируется событие `CourseCreated`.

Таким образом, операция создания курса разделяется на:
- командную часть, которая принимает запрос и публикует сообщение;
- обработчик события, который асинхронно сохраняет данные;
- модель чтения, из которой данные затем получает основной LMS API.

### Почему выбран Course

Сущность `Course` выбрана для CQRS, потому что:

- курс является одной из основных сущностей LMS;
- создание курса относится к write-операциям;
- чтение списка курсов может выполняться независимо от записи;
- курс уже участвует в связях с уроками, записью пользователей и прогрессом;
- для курса удобно показать eventual consistency: после отправки команды данные появляются в read-модели не мгновенно, а после обработки сообщения consumer-сервисом.

## 2. Проектирование Event-Driven архитектуры

В рамках ЛР 6 для LMS проектируется событийно-ориентированное взаимодействие на базе Kafka. Основной реализуемый сценарий — создание курса через CQRS.

### Event producers

Event producer — это компонент, который создаёт и публикует сообщение в брокер Kafka.

| Producer | Событие / команда | Назначение |
|---|---|---|
| Course Command Service | `CourseCreateRequested` | Принимает HTTP-запрос на создание курса и публикует команду в Kafka |
| Course Consumer Service | `CourseCreated` | После сохранения курса может сформировать событие о создании курса |
| User Service | `UserCreated` | Создаёт событие после регистрации пользователя |
| Lesson Service | `LessonAddedToCourse` | Создаёт событие после добавления урока в курс |
| Enrollment Service | `UserEnrolledToCourse` | Создаёт событие после записи пользователя на курс |
| Progress Service | `LessonCompleted` | Создаёт событие после прохождения урока пользователем |

В текущей реализации лабораторной работы физически реализуется producer для команды `CourseCreateRequested`.

### Event consumers

Event consumer — это компонент, который читает сообщения из Kafka и выполняет обработку события.

| Consumer | Обрабатываемое событие / команда | Действие |
|---|---|---|
| Course Consumer Service | `CourseCreateRequested` | Сохраняет курс в PostgreSQL и MongoDB |
| Course Service | `CourseCreated` | Использует обновлённую read-модель курсов |
| Lesson Service | `CourseCreated` | Может использовать данные курса при добавлении уроков |
| Enrollment Service | `CourseCreated` | Может использовать данные курса при записи пользователей |
| Progress Service | `LessonAddedToCourse`, `UserEnrolledToCourse`, `LessonCompleted` | Обновляет прогресс прохождения курса |

В текущей реализации лабораторной работы физически реализуется consumer для команды `CourseCreateRequested`.

### Типы событий и структура payload

Для обмена сообщениями используется единый JSON-формат.

Общая структура сообщения:

```json
{
  "event_id": "uuid",
  "event_type": "CourseCreateRequested",
  "occurred_at": "2026-05-12T12:00:00Z",
  "payload": {}
}
```

Поля сообщения:

- event_id	- Уникальный идентификатор события или команды
- event_type	- Тип события
- occurred_at	- Время формирования сообщения
- payload	- Полезная нагрузка события

Для создания курса используется событие `CourseCreateRequested`.
```json
{
  "event_id": "a7f0c5e2-5a7a-4df4-b1d1-0d4f4b4d2b1a",
  "event_type": "CourseCreateRequested",
  "occurred_at": "2026-05-12T12:00:00Z",
  "payload": {
    "title": "System Design",
    "description": "Course about software architecture",
    "teacher_id": 1
  }
}
```

### Поток событий в системе

Основной поток событий для создания курса:

1. Клиент отправляет HTTP-запрос на создание курса в `Course Command Service`.
2. `Course Command Service` валидирует входные данные.
3. `Course Command Service `формирует сообщение CourseCreateRequested.
4. Сообщение публикуется в Kafka topic `lms.course.commands`.
5. Дополнительно команда может быть сохранена в Redis как временная command-модель.
6. `Course Consumer Service` читает сообщение из Kafka.
7. `Course Consumer Service` сохраняет курс в PostgreSQL.
8. `Course Consumer Service` сохраняет агрегат курса в MongoDB.
9. После успешной обработки сообщение считается обработанным.
10. Данные становятся доступны для чтения через read-модель LMS.

Такой подход разделяет запись и чтение данных. Command-сервис не зависит от доступности базы данных напрямую в момент обработки HTTP-запроса, а сохранение курса выполняется асинхронно отдельным consumer-сервисом.

## 3. Проектирование взаимодействия через брокер сообщений

### Выбор брокера сообщений

Для реализации Event-Driven взаимодействия выбран брокер сообщений **Apache Kafka**.

Kafka используется для передачи команды создания курса от `Course Command Service` к `Course Consumer Service`.

Выбор Kafka обоснован тем, что она:

- работает с сообщениями через `topic`;
- хранит сообщения на диске;
- поддерживает повторное чтение сообщений через `offset`;
- позволяет масштабировать обработчики через `consumer group`;
- сохраняет порядок сообщений внутри одной partition;
- хорошо подходит для реализации CQRS и асинхронной обработки команд.

В рамках ЛР используется topic:

| Topic | Назначение |
|---|---|
| `lms.course.commands` | Передача команд на создание курса |

### Формат сообщений

Для сообщений выбран формат **JSON**.

Общая структура сообщения:

```json
{
  "event_id": "uuid",
  "event_type": "CourseCreateRequested",
  "occurred_at": "2026-05-12T12:00:00Z",
  "payload": {}
}
```

Сообщение для создания курса:
```json
{
  "event_id": "a7f0c5e2-5a7a-4df4-b1d1-0d4f4b4d2b1a",
  "event_type": "CourseCreateRequested",
  "occurred_at": "2026-05-12T12:00:00Z",
  "payload": {
    "title": "System Design",
    "description": "Course about software architecture",
    "teacher_id": 1
  }
}
```

### Гарантии доставки сообщений

Для взаимодействия через Kafka используется гарантия доставки `at-least-once`.

Это означает, что сообщение будет доставлено consumer-сервису минимум один раз. При сбоях или повторной обработке одно и то же сообщение может быть доставлено повторно.

Для корректной работы при `at-least-once` обработчик должен быть **идемпотентным**.

В рамках LMS это достигается за счёт:

- использования уникального event_id;
- сохранения обработанных сообщений;
- проверки существования курса перед повторной вставкой;
- фиксации Kafka offset только после успешной записи в PostgreSQL и MongoDB.

Гарантия `exactly-once` в данной лабораторной работе не используется, так как она требует более сложной настройки транзакций Kafka и согласования состояния Kafka с базами данных. Для учебной реализации достаточно `at-least-once` с идемпотентной обработкой.


## 4. Применение паттерна CQRS

### Возможность применения CQRS

В LMS паттерн CQRS применим, так как операции записи и чтения имеют разный характер.

Операции записи изменяют состояние системы:
- создание курса;
- добавление урока;
- запись пользователя на курс;
- отметка урока как пройденного.

Операции чтения только возвращают данные:
- получение списка курсов;
- получение уроков курса;
- получение курсов пользователя;
- получение прогресса пользователя.

В рамках ЛР 6 CQRS применяется для сущности `Course`.

### Разделение операций на command и query

| Тип операции | Операция | Компонент |
|---|---|---|
| Command | Создание курса | Course Command Service |
| Query | Получение списка курсов | Course Service |
| Query | Получение курса по идентификатору | Course Service |
| Query | Получение уроков курса | Lesson Service / Course Service |

### Write model

Write model отвечает за приём команд на изменение состояния.

Для создания курса используется отдельный компонент `Course Command Service`.

Он выполняет следующие действия:

1. принимает HTTP-запрос на создание курса;
2. валидирует входные данные;
3. формирует сообщение `CourseCreateRequested`;
4. публикует сообщение в Kafka topic `lms.course.commands`;
5. при необходимости сохраняет команду в Redis;
6. возвращает клиенту ответ о принятии команды.

Важно: `Course Command Service` не записывает курс напрямую в PostgreSQL или MongoDB.

### Read model

Read model используется для чтения данных.

В LMS read model хранится в:
- PostgreSQL — основная реляционная модель;
- MongoDB — документная модель курса и уроков;
- Redis — кэш часто запрашиваемых данных.

Чтение данных выполняется через основной LMS API.

### Синхронизация write и read моделей

Синхронизация выполняется через Kafka.

Поток синхронизации:

```text
Course Command Service
  -> Kafka topic: lms.course.commands
  -> Course Consumer Service
  -> PostgreSQL / MongoDB
  -> Read API
```

После получения сообщения `CourseCreateRequested` сервис `Course Consumer Service` сохраняет курс в базы данных. После этого курс становится доступен для чтения.

### Eventual consistency

CQRS приводит к eventual consistency.

Это означает, что после отправки команды курс может быть доступен для чтения не мгновенно, а после обработки сообщения consumer-сервисом.

Например:

- Клиент отправил запрос на создание курса.
- `Course Command Service` вернул ответ `202 Accepted`.
- Сообщение попало в Kafka.
- `Course Consumer Service` обработал сообщение.
- Курс появился в read model.

Такой подход повышает слабую связанность компонентов и позволяет независимо масштабировать запись и чтение.

## 5. Реализация простого Event-Driven сервиса

В рамках ЛР 6 реализуется простой Event-Driven сценарий для создания курса с применением Apache Kafka и паттерна CQRS.

Реализация выполняется на C++ с использованием фреймворка userver. Для работы с Kafka используются штатные компоненты userver: `kafka::ProducerComponent` и `kafka::ConsumerComponent`.

### Настройка Kafka через Docker

Kafka запускается как отдельный контейнер в `docker-compose.yml`.

В состав docker-compose окружения добавляются:

| Контейнер | Назначение |
|---|---|
| `kafka` | Брокер сообщений Apache Kafka |
| `zookeeper` или `kafka-controller` | Служебный компонент для работы Kafka |
| `lms_service` | Основной сервис LMS |
| `course_command_service` | userver-сервис, публикующий команды создания курса в Kafka |
| `course_consumer_service` | userver-сервис, читающий команды из Kafka и сохраняющий курс в БД |
| `postgres` | Реляционная база данных |
| `mongo` | Документная база данных для агрегата курса |
| `redis` | Кэш и временное хранилище command-модели |

Основной Kafka topic:

| Topic | Назначение |
|---|---|
| `lms.course.commands` | Передача команд на создание курса |

Дополнительно может использоваться topic:

| Topic | Назначение |
|---|---|
| `lms.course.events` | Публикация факта успешного создания курса |

### Producer

Producer реализуется в отдельном userver-сервисе `course_command_service`.

Назначение producer-сервиса:

1. принять HTTP-запрос на создание курса;
2. провалидировать входные данные;
3. сформировать сообщение `CourseCreateRequested`;
4. сохранить команду в Redis по шаблону сквозной записи;
5. отправить сообщение в Kafka topic `lms.course.commands`;
6. вернуть клиенту ответ `202 Accepted`.

Producer не записывает курс напрямую в PostgreSQL или MongoDB. Это соответствует CQRS: командная сторона только принимает команду и публикует сообщение.

Схема producer-части:

```text
Client
  -> Course Command Service
  -> Redis
  -> Kafka topic: lms.course.commands
```

### Consumer

Consumer реализуется в отдельном userver-сервисе course_consumer_service.

Назначение consumer-сервиса:

1. Подписаться на Kafka topic `lms.course.commands`;
2. Читать сообщения типа `CourseCreateRequested`;
3. Проверять, не было ли сообщение уже обработано;
4. Сохранять курс в PostgreSQL;
5. Сохранять документ курса в MongoDB;
6. Фиксировать Kafka offset только после успешной обработки сообщения.

Схема consumer-части:
```text
Kafka topic: lms.course.commands
  -> Course Consumer Service
  -> PostgreSQL
  -> MongoDB
```
Consumer является отдельным контейнером. Это соответствует уточнению к заданию: отдельный контейнер считывает сообщения из Kafka topic и сохраняет их в базу данных.

### Проверка взаимодействия

После запуска системы через Docker Compose взаимодействие проверяется следующим образом:

- отправляется HTTP-запрос на создание курса в `course_command_service`;
- сервис публикует сообщение в Kafka;
- `course_consumer_service` считывает сообщение;
- курс появляется в PostgreSQL;
- курс появляется в MongoDB;
- основной LMS API может читать созданный курс из read-модели.

## 6. Документация событий

### Kafka Topics

| Topic | Назначение |
|---|---|
| `lms.course.commands` | Команды создания курсов |
| `lms.course.events` | Доменные события LMS |


### Событие: CourseCreateRequested

### Тип сообщения

Command event

### Producer

- Course Command Service

### Consumers

- Course Consumer Service

### Topic

```text
lms.course.commands
```

### Payload
```json
{
  "event_id": "uuid",
  "event_type": "CourseCreateRequested",
  "occurred_at": "2026-05-12T12:00:00Z",
  "payload": {
    "title": "System Design",
    "description": "Course about software architecture",
    "teacher_id": 1
  }
}
```
### Delivery guarantee
- at-least-once

### Особенности обработки
- consumer проверяет event_id;
- offset фиксируется только после успешной обработки;
- курс сохраняется в PostgreSQL и MongoDB;
- Kafka используется как транспорт между command и read model.

### Реализованный сценарий
```texr
Client
  -> Course Command Service
  -> Kafka topic lms.course.commands
  -> Course Consumer Service
  -> PostgreSQL / MongoDB
```

## Запуск проекта

### 1. Перейти в каталог сервиса

```bash
cd lms_service
```
### 2. Поднять PostgreSQL
```bash
docker compose up -d postgres
```
### 3. Инициализировать PostgreSQL
```bash
docker exec -i lms_postgres psql -U postgres -d lms_db < sql/schema.sql
docker exec -i lms_postgres psql -U postgres -d lms_db < sql/data.sql
```

#### Проверка корректности заполнения PostgreSQL

Подключиться к БД внутри контейнера
```bash
docker exec -it lms_postgres psql -U postgres -d lms_db
```
Ты попадёшь в `lms_db=#`

Для проверки количества записей в таблицах выполнить запрос:

```sql
SELECT 'users' AS table_name, COUNT(*) FROM users
UNION ALL
SELECT 'courses', COUNT(*) FROM courses
UNION ALL
SELECT 'lessons', COUNT(*) FROM lessons
UNION ALL
SELECT 'course_enrollments', COUNT(*) FROM course_enrollments
UNION ALL
SELECT 'lesson_progress', COUNT(*) FROM lesson_progress;
```

Результат должен показать, что в каждой таблице содержится по 10 записей,
что соответствует требованиям задания.

### 4. Поднять MongoDB
```bash
docker compose up -d mongo
```

#### Проверка корректности заполнения MongoDB
Подключиться к MongoDB:
```bash
docker exec -it lms_mongo mongosh
```

Внутри mongosh выполнить:
```bash
use lms_mongo_db
show collections
db.courses.countDocuments()
db.courses.findOne()
db.courses.getIndexes()
```

Ожидается, что коллекция courses существует и содержит 10 документов.

### 5. Поднять Redis
```bash
docker compose up -d redis
```

Проверка Redis
```
docker exec -it lms_redis redis-cli ping
```

Ожидаемый результат:
```
PONG
```

### 6. Поднять Kafka

```bash
docker compose up -d zookeeper kafka
```

Проверка Kafka:
```bash
docker exec -it lms_kafka kafka-topics \
--bootstrap-server kafka:9092 \
--list
```
### 7. Запустить producer и consumer
```bash
docker compose up --build \
lms-service \
course-command-service \
course-consumer-service
```
### 8. Запустить API в Docker
```bash
docker compose up --build lms-service
```
После запуска API будет доступно по адресу: `http://localhost:8080`

### 9. Запуск тестов в Dev Container
```bash
make test-debug
```
