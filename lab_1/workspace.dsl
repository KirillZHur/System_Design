workspace "LMS" "Система управления обучением"  {
  !identifiers hierarchical

  model {

    guest = person "Гость" "Неавторизованный пользователь системы"
    student = person "Студент" "Пользователь, проходящий обучение на платформе"
    teacher = person "Преподаватель" "Пользователь, создающий образовательный контент"
    admin = person "Администратор" "Пользователь, управляющий системой и учетными записями"

    emailSystem = softwareSystem "Email-сервис" "Система отправки писем подтверждения регистрации и уведомлений" {
      tags "External"
    }

    lms = softwareSystem "LMS" "Система управления обучением" {

      spa = container "Web Application" "Пользовательский интерфейс для студентов и преподавателей" "Angular (SPA)" {
        tags "WebApp"
      }

      apiGateway = container "API Gateway" "Единая точка входа для маршрутизации запросов, проверки JWT" "NGINX / Kong" {
        tags "Gateway"
      }

      userService = container "User Service" "Сервис регистрации, поиска пользователей, управления учетными записями" "C++ / userver (REST)"

      courseService = container "Course Service" "Сервис создания курсов, получения списка курсов, добавления уроков и получения уроков курса" "C++ / userver (REST)"

      enrollmentService = container "Enrollment Service" "Сервис записи пользователей на курсы, получения курсов пользователя и отметки о прохождении уроков" "C++ / userver (REST)"

      db = container "Database" "База данных пользователей, курсов, урокоу, записей на курсы и хранения прогресса уроков" "PostgreSQL" {
        tags "Database"
      }

    }

    guest -> lms "Просматривает курсы" "HTTPS"
    student -> lms "Проходит обучение" "HTTPS"
    teacher -> lms "Создает курсы и уроки" "HTTPS"
    admin -> lms "Администрирует систему" "HTTPS"

    lms -> emailSystem "Отправляет email уведомления" "SMTP"

    guest -> lms.spa "Просматривает курсы" "HTTPS"
    student -> lms.spa "Проходит обучение" "HTTPS"
    teacher -> lms.spa "Создает курсы и уроки" "HTTPS"
    admin -> lms.spa "Управляет системой" "HTTPS"

    lms.spa -> lms.apiGateway "Выполняет API-запросы" "HTTPS/REST"

    lms.apiGateway -> lms.userService "Маршрутизирует /users" "HTTPS/REST"
    lms.apiGateway -> lms.courseService "Маршрутизирует /courses и /lessons" "HTTPS/REST"
    lms.apiGateway -> lms.enrollmentService "Маршрутизирует /enrollments и /progress" "HTTPS/REST"

    lms.userService -> lms.db "Читает и записывает пользователей" "SQL"
    lms.userService -> emailSystem "Отправляет подтверждение регистрации" "SMTP"

    lms.courseService -> lms.db "Читает и записывает курсы и уроки" "SQL"

    lms.enrollmentService -> lms.db "Читает и записывает записи на курсы и прогресс обучения" "SQL"
    lms.enrollmentService -> emailSystem "Отправляет уведомление о записи на курс/прохождение урока" "SMTP"

  }

  views {

    systemContext lms "SystemContext" "Диаграмма контекста системы (С1)"  {
      include *
      autolayout lr
    }

    container lms "Containers" "Диаграмма контейнеров системы (C2)" {
      include *
      autolayout lr
    }

    dynamic lms "Add_Student_At_Course" "Запись пользователя на курс" {

      autolayout lr

      student -> lms.spa "Открывает страницу курса"
      lms.spa -> lms.apiGateway "POST /enrollments (JWT)"
      lms.apiGateway -> lms.enrollmentService "Перенаправляет запрос"
      lms.enrollmentService -> lms.db "INSERT enrollment"
    }

    styles {

      element "Person" {
        shape person
        background #08427b
        color #ffffff
      }

      element "Software System" {
        background #1168bd
        color #ffffff
      }

      element "External" {
        background #999999
        color #ffffff
      }

      element "Container" {
        background #438dd5
        color #ffffff
      }

      element "WebApp" {
        shape webbrowser
      }

      element "Gateway" {
        shape hexagon
      }

      element "Database" {
        shape cylinder
      }

    }

  }

}