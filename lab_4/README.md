# Задание №4 по предмету System Design

## Выполнил: Журавлев Кирилл М8О-102СВ-25

## Вариант 19: Система управления обучением

## 1. Проектирование документой модели

### Выбор сущностей для переноса в MongoDB

В предыдущей лабораторной работе система LMS была реализована с использованием реляционной базы данных PostgreSQL. Основные сущности системы:

- Пользователь (users)
- Курсы (courses)
- Уроки (lessons)
- Записи на курсы (course_enrollments)
- Прогресс по урокам (lesson_progress)

В рамках данной лабораторной работы было принято решение перенести в MongoDB следующие сущности:

- Курсы (courses)
- Уроки (lessons)

При этом сущность пользователей (users), а также связанные с ней данные (аутентификация, прогресс) остаются в PostgreSQL.

#### Обоснование выбора

- Пользователи являются критическими клиентскими данными и требуют строгой консистентности и транзакционност, поэтому остаются в PostgreSQL
- Курсы и уроки представляют собой иерархическую структуру, поэтому хорошо подходят для документной модели
- Курсы чаще всего читаются вместе с уроками,а значит удобно хранить их в одном документе
- MongoDB позволяет эффективно хранить вложенные структуры


### Выбор коллекций

В MongoDB была спроектирована следующая коллекция:

- `courses`

Дополнительно отдельная коллекция для уроков **не создается**, так как уроки логически принадлежат курсу и не используются независимо.


### Структура документа

Документ коллекции `courses` имеет следующую структуру:

```json
{
  "_id": 1,
  "title": "Основы программирования",
  "description": "Базовый курс",
  "teacher_id": 10,
  "created_at": ISODate("2026-04-13T10:00:00Z"),
  "lessons": [
    {
      "id": 1,
      "title": "Введение",
      "content": "Содержимое урока",
      "position": 1,
      "created_at": ISODate("2026-04-13T10:00:00Z")
    },
    {
      "id": 2,
      "title": "Переменные",
      "content": "Содержимое урока",
      "position": 2,
      "created_at": ISODate("2026-04-13T11:00:00Z")
    }
  ]
}
```

### Embedded documents и References

В разработанной модели используются оба подхода:

#### Embedded documents (вложенные документы)
- Уроки (lessons) хранятся внутри документа курса
- Каждый курс содержит массив уроков
#### References (ссылки)
- Связь с пользователем реализована через поле teacher_id
- Пользователь хранится в PostgreSQL и не дублируется в MongoDB

### Обоснование использования embedded

Выбор вложенной модели для уроков обусловлен следующими причинами:

1. Логическая зависимость
- Урок не существует без курса
2. Совместное использование данных
- При получении курса обычно требуется список всех уроков
3. Снижение количества запросов
- Один запрос вместо нескольких (без join)
4. Производительность
- Все данные курса загружаются одним чтением

### Обоснование использования reference

Поле teacher_id хранится как ссылка, так как:

- Пользователи находятся в PostgreSQL
- Требуется разделение ответственности между БД
- Нет необходимости дублировать пользовательские данные в MongoDB

## 2. Создание базы данных и коллекций

### Создание MongoDB

Для работы с базой данных MongoDB был использован Docker, что позволяет
обеспечить изолированную и воспроизводимую среду выполнения.

Для запуска базы данных был создан сервис `mongo` в файле `docker-compose.yml`:

```yaml
services:
  mongo:
    image: mongo:5.0
    container_name: lms_mongo
    restart: unless-stopped
    ports:
      - "27017:27017"
    volumes:
      - mongo_data:/data/db
      - ./mongo:/docker-entrypoint-initdb.d
    healthcheck:
      test:
        [
          "CMD-SHELL",
          "echo 'db.runCommand({ ping: 1 }).ok' | mongosh localhost:27017/test --quiet"
        ]
      interval: 5s
      timeout: 5s
      retries: 10

volumes:
  mongo_data:
```

### Создание коллекции

В рамках лабораторной работы используется MongoDB база данных `lms_mongo_db`.

Для хранения данных была создана коллекция:

- `courses`

Данная коллекция содержит документы курсов, внутри которых хранятся вложенные документы уроков.


#### Структура коллекции

Каждый документ коллекции `courses` содержит:
- Идентификатор курса
- Название курса
- Описание курса
- Идентификатор преподавателя
- Дату создания курса
- Массив вложенных документов `lessons`

Каждый вложенный документ `lesson` содержит:
- Идентификатор урока
- Название урока
- Содержимое урока
- Порядковый номер
- Дату создания урока

### Добавление тестовых данных

Для заполнения базы данных подготовлен файл [data.js](lms_service/mongo/data.js).

В данном файле выполняется:
- выбор базы данных `lms_mongo_db`;
- вставка тестовых документов в коллекцию `courses`.

В коллекцию было добавлено 10 документов курсов,каждый курс содержит по 2 вложенных урока.

### Используемые типы данных MongoDB

При заполнении коллекции были использованы следующие типы данных MongoDB:

- `Text` — названия курсов, описания курсов, названия уроков, содержимое уроков
- `Number` — идентификаторы курсов, преподавателей, уроков, а также поле `position`
- `ISODate("...")` — дата создания курса и уроков
- `Array` — массив `lessons`
- `Embedded Document` — каждый урок внутри массива `lessons`
- `ObjectId(...)` — может использоваться MongoDB автоматически в качестве идентификатора документа

## 3. Реализация CRUD операций

### Операции Create

На этапе создания данных были реализованы следующие операции:

- Добавление нового курса
- Добавление нового урока в существующий курс

Для добавления нового курса используется операция `insertOne()`.  
Для добавления урока в массив `lessons` используется оператор `$push`.

### Операции Read

Для чтения данных были реализованы следующие запросы:

- Получение списка всех курсов
- Получение курса по id
- Поиск курсов по преподавателю
- Поиск курсов по названию
- Поиск курсов по нескольким условиям
- Поиск курса, содержащего урок с определенным номером

При выполнении запросов используются операторы:
- `$eq`
- `$ne`
- `$gt`
- `$lt`
- `$in`
- `$and`
- `$or`

### Операции Update

Для изменения данных были реализованы:

- Обновление названия курса
- Обновление описания курса
- Изменение данных вложенного урока

Для этих операций используются оператор `$set`.

### Операции Delete

Для удаления данных были реализованы:

- Удаление курса
- Удаление урока из массива `lessons`

Для удаления вложенного урока используется оператор `$pull`.

### Особенности реализации

Так как уроки хранятся внутри документа курса, операции с уроками выполняются через обновление массива `lessons`, а не через отдельную коллекцию.

Это соответствует выбранной документной модели и позволяет выполнять основные действия с курсами и уроками без использования JOIN-операций.

MongoDB запросы для всех операций находятся в файле [queries.js](lms_service/mongo/queries.js).

## 4. Валидация схем

### Выбор коллекции для валидации

В рамках лабораторной работы валидация была реализована для коллекции:

- `courses`

Данная коллекция является основной в разработанной документной модели и содержит как данные курса, так и вложенные документы уроков.

### Проверяемые поля

Для документа курса были определены обязательные поля:

- `_id`
- `title`
- `teacher_id`
- `created_at`
- `lessons`

Для вложенного документа урока обязательными являются:

- `id`
- `title`
- `position`
- `created_at`

### Ограничения схемы

В схеме были заданы следующие ограничения:

- `title` курса должен быть строкой длиной не менее 2 символов;
- `description` может быть строкой или `null`;
- `teacher_id` должен быть целым числом;
- `created_at` должен быть датой;
- `lessons` должен быть массивом;
- каждый урок должен быть объектом;
- `title` урока должен быть строкой длиной не менее 2 символов;
- `position` должен быть целым числом больше 0;
- `content` может быть строкой или `null`;
- `created_at` урока должен быть датой.

### Реализация

Валидация была оформлена в файле [validation.js](lms_service/mongo/validation.js)

Для задания схемы используется оператор `collMod` и валидатор `$jsonSchema`.

### Проверка работы валидации

Для проверки корректности работы схемы была выполнена попытка вставки невалидного документа, содержащего ошибки в типах и обязательных полях.

Ожидаемый результат:
- MongoDB отклоняет вставку документа
- В ответе возвращается сообщение об ошибке валидации

```bash
lms_mongo_db> db.courses.insertOne({
...   _id: NumberInt(101),
...   title: "Корректный курс",
...   description: "Проверка валидного документа",
...   teacher_id: NumberInt(2),
...   created_at: ISODate("2026-04-13T13:00:00Z"),
...   lessons: [
...     {
...       id: NumberInt(1),
...       title: "Корректный урок",
...       content: "Тест",
...       position: NumberInt(1),
...       created_at: ISODate("2026-04-13T13:10:00Z")
...     }
...   ]
... })
{ acknowledged: true, insertedId: Int32(101) }
lms_mongo_db> db.courses.insertOne({
...   _id: "bad_id",
...   title: "A",
...   teacher_id: "wrong_teacher",
...   created_at: "not_a_date",
...   lessons: [
...     {
...       id: "bad_lesson",
...       title: "",
...       position: 0,
...       created_at: "wrong_date"
...     }
...   ]
... })
Uncaught:
MongoServerError: Document failed validation
Additional information: {
  failingDocumentId: 'bad_id',
  details: {
    operatorName: '$jsonSchema',
    schemaRulesNotSatisfied: [
      {
        operatorName: 'properties',
        propertiesNotSatisfied: [
          {
            propertyName: '_id',
            details: [
              {
                operatorName: 'bsonType',
                specifiedAs: { bsonType: 'int' },
                reason: 'type did not match',
                consideredValue: 'bad_id',
                consideredType: 'string'
              }
            ]
          },
          {
            propertyName: 'title',
            details: [
              {
                operatorName: 'minLength',
                specifiedAs: { minLength: 2 },
                reason: 'specified string length was not satisfied',
                consideredValue: 'A'
              }
            ]
          },
          {
            propertyName: 'teacher_id',
            details: [
              {
                operatorName: 'bsonType',
                specifiedAs: { bsonType: 'int' },
                reason: 'type did not match',
                consideredValue: 'wrong_teacher',
                consideredType: 'string'
              }
            ]
          },
          {
            propertyName: 'created_at',
            details: [
              {
                operatorName: 'bsonType',
                specifiedAs: { bsonType: 'date' },
                reason: 'type did not match',
                consideredValue: 'not_a_date',
                consideredType: 'string'
              }
            ]
          },
          {
            propertyName: 'lessons',
            details: [
              {
                operatorName: 'items',
                reason: 'At least one item did not match the sub-schema',
                itemIndex: 0,
                details: [
                  {
                    operatorName: 'properties',
                    propertiesNotSatisfied: [
                      {
                        propertyName: 'id',
                        details: [
                          {
                            operatorName: 'bsonType',
                            specifiedAs: { bsonType: 'int' },
                            reason: 'type did not match',
                            consideredValue: 'bad_lesson',
                            consideredType: 'string'
                          }
                        ]
                      },
                      {
                        propertyName: 'title',
                        details: [
                          {
                            operatorName: 'minLength',
                            specifiedAs: { minLength: 2 },
                            reason: 'specified string length was not satisfied',
                            consideredValue: ''
                          }
                        ]
                      },
                      {
                        propertyName: 'position',
                        details: [
                          {
                            operatorName: 'minimum',
                            specifiedAs: { minimum: 1 },
                            reason: 'comparison failed',
                            consideredValue: 0
                          }
                        ]
                      },
                      {
                        propertyName: 'created_at',
                        details: [
                          {
                            operatorName: 'bsonType',
                            specifiedAs: { bsonType: 'date' },
                            reason: 'type did not match',
                            consideredValue: 'wrong_date',
                            consideredType: 'string'
                          }
                        ]
                      }
                    ]
                  }
                ]
              }
            ]
          }
        ]
      }
    ]
  }
}
```

## 5. Агрегации (опционально)

### Постановка задачи

В рамках лабораторной работы был реализован aggregation pipeline, позволяющий получить список курсов определенного преподавателя с вычислением количества уроков в каждом курсе.

Такой запрос полезен для анализа структуры учебных курсов и демонстрирует возможности MongoDB по обработке и преобразованию данных.

### Используемые стадии aggregation pipeline

В pipeline были использованы следующие стадии:

- `$match` — фильтрация курсов по идентификатору преподавателя
- `$project` — выбор нужных полей и вычисление количества уроков
- `$sort` — сортировка курсов по количеству уроков

### Пример агрегации

Пример aggregation pipeline:

```javascript
db.courses.aggregate([
  {
    $match: {
      teacher_id: NumberInt(1)
    }
  },
  {
    $project: {
      _id: 1,
      title: 1,
      teacher_id: 1,
      lessons_count: { $size: "$lessons" }
    }
  },
  {
    $sort: {
      lessons_count: -1,
      title: 1
    }
  }
])
```

Данный pipeline:

- Отбирает курсы преподавателя с teacher_id = 1
- Вычисляет количество уроков в каждом курсе
- Сортирует результат по убыванию количества уроков

Вывод примера:
```bash
[
  {
    _id: 6,
    title: 'Объектно-ориентированное программирование',
    teacher_id: 1,
    lessons_count: 2
  },
  {
    _id: 1,
    title: 'Основы программирования',
    teacher_id: 1,
    lessons_count: 2
  },
  { _id: 100, title: 'Тестовый курс', teacher_id: 1, lessons_count: 1 }
]
```

## 6. Индексы

Для ускорения выполнения запросов были созданы индексы:

- по полю teacher_id (поиск курсов преподавателя)
- по полю title (поиск по названию курса)
- по вложенному полю lessons.position

Создание индексов происходит в файле [indexes.js](lms_service/mongo/indexes.js)

## 7. Подключение базы данных MongoDB к API

На данном этапе в API были переведены сущности `courses` и `lessons`, которые теперь хранятся в MongoDB.

Для этого в приложение был добавлен компонент `mongo-db`, использующий MongoDB через `userver::components::Mongo`.

В слое доступа к данным был реализован отдельный класс `MongoStorage`, через который выполняются следующие операции:

- Создание курса
- Получение списка курсов
- Добавление урока в курс
- Получение списка уроков курса

При этом остальные сущности системы продолжают использовать PostgreSQL:

- Пользователи
- Аутентификация
- Записи на курсы
- Прогресс по урокам

Таким образом, в сервисе используется комбинированная схема хранения:
- PostgreSQL — для клиентских и транзакционных данных
- MongoDB — для документного хранения курсов и уроков

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

### 6. Запустить API в Docker
```bash
docker compose up --build lms-service
```
После запуска API будет доступно по адресу: `http://localhost:8080`

### 7. Запуск тестов
```bash
make test-debug
```
