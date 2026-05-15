#pragma once
#include <ostream>
#include <string>
#include <string_view>
#include <iomanip>

class Explainer {
public:
    static Explainer& get() {
        static Explainer instance;
        return instance;
    }

    void enable(std::ostream& os) {
        enabled_ = true;
        os_      = &os;
    }

    bool active() const { return enabled_; }

    void log(std::string_view pass_tag,
             std::string_view fn_name,
             std::string_view msg) const {
        if (!enabled_ || !os_) return;

        std::string full_tag = "[" + std::string(pass_tag) + "]";

        *os_ << std::left << std::setw(12) << full_tag
             << "@"  << fn_name << ": "
             << msg << "\n";
    }

private:
    bool          enabled_ = false;
    std::ostream* os_      = nullptr;

    Explainer() = default;
    Explainer(const Explainer&)            = delete;
    Explainer& operator=(const Explainer&) = delete;
};
