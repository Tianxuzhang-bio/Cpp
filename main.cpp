#if __has_include(<QApplication>)
#include <QApplication>
#include <QComboBox>
#include <QDateTime>
#include <QDebug>
#include <QDoubleValidator>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

class CalculatorWidget : public QWidget {
public:
    CalculatorWidget(QWidget *parent = nullptr)
        : QWidget(parent)
        , lhsInput(new QLineEdit(this))
        , rhsInput(new QLineEdit(this))
        , operatorSelector(new QComboBox(this))
        , resultLabel(new QLabel(tr("Result: --"), this))
        , errorLabel(new QLabel(this))
        , logView(new QPlainTextEdit(this)) {
        setWindowTitle(tr("Qt Calculator"));
        setMinimumSize(320, 180);

        auto *validator = new QDoubleValidator(this);
        validator->setNotation(QDoubleValidator::StandardNotation);

        lhsInput->setPlaceholderText(tr("Left operand"));
        lhsInput->setValidator(validator);
        rhsInput->setPlaceholderText(tr("Right operand"));
        rhsInput->setValidator(new QDoubleValidator(this));

        operatorSelector->addItems({"+", "-", "×", "÷"});
        operatorSelector->setCurrentIndex(0);

        auto *computeButton = new QPushButton(tr("Compute"), this);
        auto *clearLogButton = new QPushButton(tr("Clear Log"), this);

        errorLabel->setStyleSheet("color: #b00020;");
        errorLabel->setVisible(false);

        logView->setReadOnly(true);
        logView->setPlaceholderText(tr("计算日志将在此显示。"));

        auto *inputLayout = new QHBoxLayout;
        inputLayout->addWidget(lhsInput);
        inputLayout->addWidget(operatorSelector);
        inputLayout->addWidget(rhsInput);

        auto *buttonLayout = new QHBoxLayout;
        buttonLayout->addStretch();
        buttonLayout->addWidget(computeButton);
        buttonLayout->addWidget(clearLogButton);

        auto *layout = new QVBoxLayout(this);
        layout->addLayout(inputLayout);
        layout->addLayout(buttonLayout);
        layout->addWidget(resultLabel);
        layout->addWidget(errorLabel);
        layout->addWidget(logView);
        layout->addStretch();

        connect(computeButton, &QPushButton::clicked, this, &CalculatorWidget::calculate);
        connect(clearLogButton, &QPushButton::clicked, logView, &QPlainTextEdit::clear);
    }

private:
    void calculate() {
        errorLabel->setVisible(false);

        bool lhsOk = false;
        bool rhsOk = false;
        const double lhsValue = lhsInput->text().toDouble(&lhsOk);
        const double rhsValue = rhsInput->text().toDouble(&rhsOk);

        if (!lhsOk || !rhsOk) {
            showError(tr("请输入有效的数字。"));
            appendLog(tr("调试: 输入无法解析 -> 左值：%1，右值：%2")
                          .arg(lhsInput->text())
                          .arg(rhsInput->text()));
            return;
        }

        const QString op = operatorSelector->currentText();
        double result = 0.0;

        if (op == "+") {
            result = lhsValue + rhsValue;
        } else if (op == "-") {
            result = lhsValue - rhsValue;
        } else if (op == "×") {
            result = lhsValue * rhsValue;
        } else if (op == "÷") {
            if (rhsValue == 0.0) {
                showError(tr("除数不能为0。"));
                appendLog(tr("调试: 尝试除以零，左值：%1")
                              .arg(lhsValue));
                return;
            }
            result = lhsValue / rhsValue;
        }

        resultLabel->setText(tr("Result: %1").arg(result));

        const QString logMessage = tr("%1 | %2 %3 %4 = %5")
                                       .arg(QDateTime::currentDateTime().toString(Qt::ISODate))
                                       .arg(lhsValue)
                                       .arg(op)
                                       .arg(rhsValue)
                                       .arg(result);
        appendLog(logMessage);
    }

    void showError(const QString &message) {
        errorLabel->setText(message);
        errorLabel->setVisible(true);
        resultLabel->setText(tr("Result: --"));
        appendLog(tr("错误: %1").arg(message));
    }

    void appendLog(const QString &message) {
        qDebug() << "Calculator log:" << message;
        logView->appendPlainText(message);
    }

    QLineEdit *lhsInput;
    QLineEdit *rhsInput;
    QComboBox *operatorSelector;
    QLabel *resultLabel;
    QLabel *errorLabel;
    QPlainTextEdit *logView;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    CalculatorWidget calculator;
    calculator.show();
    return app.exec();
}

#else

#include <cstdlib>
#include <iostream>
#include <limits>
#include <optional>
#include <string>

namespace {

std::optional<double> readNumber(const std::string &prompt) {
    std::cout << prompt;
    double value = 0.0;
    if (!(std::cin >> value)) {
        return std::nullopt;
    }
    return value;
}

std::optional<char> readOperator() {
    std::cout << "请输入运算符 (+, -, *, /): ";
    char op = 0;
    if (!(std::cin >> op)) {
        return std::nullopt;
    }
    switch (op) {
    case '+':
    case '-':
    case '*':
    case '/':
        return op;
    default:
        std::cout << "无效的运算符。\n";
        return std::nullopt;
    }
}

void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

} // namespace

int main() {
    std::cout << "简单计算器 (输入 q 退出)\n";

    while (true) {
        std::cout << "输入表达式: ";

        std::string firstToken;
        if (!(std::cin >> firstToken)) {
            std::cout << "输入错误，程序退出。\n";
            return EXIT_FAILURE;
        }

        if (firstToken == "q" || firstToken == "Q") {
            std::cout << "再见！\n";
            break;
        }

        char *endPtr = nullptr;
        double lhs = std::strtod(firstToken.c_str(), &endPtr);
        if (endPtr == firstToken.c_str() || *endPtr != '\0') {
            std::cout << "请输入有效的数字。\n";
            clearInput();
            continue;
        }

        const auto op = readOperator();
        if (!op) {
            clearInput();
            continue;
        }

        const auto rhsInput = readNumber("请输入第二个数字: ");
        if (!rhsInput) {
            std::cout << "请输入有效的数字。\n";
            clearInput();
            continue;
        }

        const double rhs = *rhsInput;

        double result = 0.0;
        switch (*op) {
        case '+':
            result = lhs + rhs;
            break;
        case '-':
            result = lhs - rhs;
            break;
        case '*':
            result = lhs * rhs;
            break;
        case '/':
            if (rhs == 0.0) {
                std::cout << "除数不能为0。\n";
                continue;
            }
            result = lhs / rhs;
            break;
        default:
            std::cout << "未知错误。\n";
            continue;
        }

        std::cout << "结果: " << result << "\n";
    }

    return EXIT_SUCCESS;
}

#endif
