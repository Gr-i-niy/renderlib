#include "core/View.h"

#include <memory>
#include <utility>

#include "core/ViewImpl.h"

IView::Ptr createView(IModel::Ptr model) {
    return std::make_unique<ViewImpl>(std::move(model));
}