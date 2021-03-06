/*  Copyright 2013 MaidSafe.net limited

    This MaidSafe Software is licensed to you under (1) the MaidSafe.net Commercial License,
    version 1.0 or later, or (2) The General Public License (GPL), version 3, depending on which
    licence you accepted on initial access to the Software (the "Licences").

    By contributing code to the MaidSafe Software, or to this project generally, you agree to be
    bound by the terms of the MaidSafe Contributor Agreement, version 1.0, found in the root
    directory of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also
    available at: http://www.maidsafe.net/licenses

    Unless required by applicable law or agreed to in writing, the MaidSafe Software distributed
    under the GPL Licence is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
    OF ANY KIND, either express or implied.

    See the Licences for the specific language governing permissions and limitations relating to
    use of the MaidSafe Software.                                                                 */

#include "maidsafe/nfs/client/messages.h"

#include <cstdint>

#include "maidsafe/nfs/utils.h"
#include "maidsafe/nfs/client/messages.pb.h"

namespace maidsafe {

namespace nfs_client {

namespace {

maidsafe_error GetError(int error_value, const std::string& error_category_name) {
  if (error_category_name == std::string(GetCommonCategory().name()))
    return MakeError(static_cast<CommonErrors>(error_value));
  if (error_category_name == std::string(GetAsymmCategory().name()))
    return MakeError(static_cast<AsymmErrors>(error_value));
  if (error_category_name == std::string(GetPassportCategory().name()))
    return MakeError(static_cast<PassportErrors>(error_value));
  if (error_category_name == std::string(GetNfsCategory().name()))
    return MakeError(static_cast<NfsErrors>(error_value));
  if (error_category_name == std::string(GetRoutingCategory().name()))
    return MakeError(static_cast<RoutingErrors>(error_value));
  if (error_category_name == std::string(GetDriveCategory().name()))
    return MakeError(static_cast<DriveErrors>(error_value));
  if (error_category_name == std::string(GetVaultCategory().name()))
    return MakeError(static_cast<VaultErrors>(error_value));
  if (error_category_name == std::string(GetApiCategory().name()))
    return MakeError(static_cast<ApiErrors>(error_value));

  BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));
}

}  // unnamed namespace

// ==================== ReturnCode =================================================================
ReturnCode::ReturnCode() : value(CommonErrors::success) {}

ReturnCode::ReturnCode(const ReturnCode& other) : value(other.value) {}

ReturnCode::ReturnCode(ReturnCode&& other) : value(std::move(other.value)) {}

ReturnCode& ReturnCode::operator=(ReturnCode other) {
  swap(*this, other);
  return *this;
}

ReturnCode::ReturnCode(const std::string& serialised_copy)
    : value([&serialised_copy] {
        protobuf::ReturnCode proto_copy;
        if (!proto_copy.ParseFromString(serialised_copy)) {
          LOG(kError) << "ReturnCode parsing error";
          BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));
        }
        return GetError(proto_copy.error_value(), proto_copy.error_category_name());
      }()) {}

std::string ReturnCode::Serialise() const {
  protobuf::ReturnCode proto_copy;
  proto_copy.set_error_value(value.code().value());
  proto_copy.set_error_category_name(value.code().category().name());
  return proto_copy.SerializeAsString();
}

bool operator==(const ReturnCode& lhs, const ReturnCode& rhs) {
  return lhs.value.code() == rhs.value.code();
}

void swap(ReturnCode& lhs, ReturnCode& rhs) MAIDSAFE_NOEXCEPT {
  using std::swap;
  swap(lhs.value, rhs.value);
}

// ==================== AvailableSizeAndReturnCode =================================================
AvailableSizeAndReturnCode::AvailableSizeAndReturnCode() : available_size(0), return_code() {}

AvailableSizeAndReturnCode::AvailableSizeAndReturnCode(uint64_t size,
                                                       const ReturnCode& return_code_in)
    : available_size(size), return_code(return_code_in) {}

AvailableSizeAndReturnCode::AvailableSizeAndReturnCode(const AvailableSizeAndReturnCode& other)
    : available_size(other.available_size), return_code(other.return_code) {}

AvailableSizeAndReturnCode::AvailableSizeAndReturnCode(AvailableSizeAndReturnCode&& other)
    : available_size(std::move(other.available_size)), return_code(std::move(other.return_code)) {}

AvailableSizeAndReturnCode& AvailableSizeAndReturnCode::operator=(
    AvailableSizeAndReturnCode other) {
  swap(*this, other);
  return *this;
}

AvailableSizeAndReturnCode::AvailableSizeAndReturnCode(const std::string& serialised_copy)
    : available_size(0), return_code() {
  protobuf::AvailableSizeAndReturnCode proto_copy;
  if (!proto_copy.ParseFromString(serialised_copy)) {
    LOG(kError) << "can't parse AvailableSizeAndReturnCode from incoming string";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));
  }
  available_size = nfs_vault::AvailableSize(proto_copy.serialised_available_size());
  return_code = ReturnCode(proto_copy.serialised_return_code());
}

std::string AvailableSizeAndReturnCode::Serialise() const {
  protobuf::AvailableSizeAndReturnCode proto_copy;
  proto_copy.set_serialised_available_size(available_size.Serialise());
  proto_copy.set_serialised_return_code(return_code.Serialise());
  return proto_copy.SerializeAsString();
}

bool operator==(const AvailableSizeAndReturnCode& lhs, const AvailableSizeAndReturnCode& rhs) {
  return lhs.available_size == rhs.available_size &&
         lhs.return_code == rhs.return_code;
}

void swap(AvailableSizeAndReturnCode& lhs, AvailableSizeAndReturnCode& rhs) MAIDSAFE_NOEXCEPT {
  swap(lhs.available_size, rhs.available_size);
  swap(lhs.return_code, rhs.return_code);
}

// ==================== DataNameAndReturnCode ======================================================
DataNameAndReturnCode::DataNameAndReturnCode() : name(), return_code() {}

DataNameAndReturnCode::DataNameAndReturnCode(nfs_vault::DataName data_name, ReturnCode code)
    : name(std::move(data_name)), return_code(std::move(code)) {}

DataNameAndReturnCode::DataNameAndReturnCode(const DataNameAndReturnCode& other)
    : name(other.name), return_code(other.return_code) {}

DataNameAndReturnCode::DataNameAndReturnCode(DataNameAndReturnCode&& other)
    : name(std::move(other.name)), return_code(std::move(other.return_code)) {}

DataNameAndReturnCode& DataNameAndReturnCode::operator=(DataNameAndReturnCode other) {
  swap(*this, other);
  return *this;
}

DataNameAndReturnCode::DataNameAndReturnCode(const std::string& serialised_copy)
    : name(), return_code() {
  protobuf::DataNameAndReturnCode proto_copy;
  if (!proto_copy.ParseFromString(serialised_copy))
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));
  name = nfs_vault::DataName(proto_copy.serialised_name());
  return_code = ReturnCode(proto_copy.serialised_return_code());
}

std::string DataNameAndReturnCode::Serialise() const {
  protobuf::DataNameAndReturnCode proto_copy;
  proto_copy.set_serialised_name(name.Serialise());
  proto_copy.set_serialised_return_code(return_code.Serialise());
  return proto_copy.SerializeAsString();
}

bool operator==(const DataNameAndReturnCode& lhs, const DataNameAndReturnCode& rhs) {
  return lhs.name == rhs.name && lhs.return_code == rhs.return_code;
}

void swap(DataNameAndReturnCode& lhs, DataNameAndReturnCode& rhs) MAIDSAFE_NOEXCEPT {
  using std::swap;
  swap(lhs.name, rhs.name);
  swap(lhs.return_code, rhs.return_code);
}

// ==================== DataNamesAndReturnCode =====================================================
DataNamesAndReturnCode::DataNamesAndReturnCode(const ReturnCode& code)
    : names(),
      return_code(code) {}

DataNamesAndReturnCode::DataNamesAndReturnCode(const std::vector<nfs_vault::DataName>& data_names,
                                               const ReturnCode& code)
    : names(),
      return_code(code) {
  for (auto data_name : data_names)
    names.insert(data_name);
}

DataNamesAndReturnCode::DataNamesAndReturnCode(const DataNamesAndReturnCode& other)
    : names(other.names), return_code(other.return_code) {}

DataNamesAndReturnCode::DataNamesAndReturnCode(DataNamesAndReturnCode&& other)
    : names(std::move(other.names)), return_code(std::move(other.return_code)) {}

DataNamesAndReturnCode& DataNamesAndReturnCode::operator=(DataNamesAndReturnCode other) {
  swap(*this, other);
  return *this;
}

void DataNamesAndReturnCode::AddDataName(const DataTagValue& tag_value, const Identity& identity) {
  names.insert(nfs_vault::DataName(tag_value, identity));
}

DataNamesAndReturnCode::DataNamesAndReturnCode(const std::string& serialised_copy)
    : names(),
      return_code() {
  protobuf::DataNamesAndReturnCode names_proto;
  if (!names_proto.ParseFromString(serialised_copy))
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));
  for (auto index(0); index < names_proto.serialised_name_size(); ++index)
    names.insert(nfs_vault::DataName(std::string(names_proto.serialised_name(index))));
  return_code = nfs_client::ReturnCode(names_proto.serialised_return_code());
}

std::string DataNamesAndReturnCode::Serialise() const {
  protobuf::DataNamesAndReturnCode names_proto;
  names_proto.set_serialised_return_code(return_code.Serialise());
  for (const auto& name : names) {
    auto name_proto(names_proto.add_serialised_name());
    *name_proto = name.Serialise();
  }
  return names_proto.SerializeAsString();
}

bool operator==(const DataNamesAndReturnCode& lhs, const DataNamesAndReturnCode& rhs) {
  return  lhs.return_code == rhs.return_code && lhs.names == rhs.names;
}

void swap(DataNamesAndReturnCode& lhs, DataNamesAndReturnCode& rhs) MAIDSAFE_NOEXCEPT {
  using std::swap;
  swap(lhs.return_code, rhs.return_code);
  swap(lhs.names, rhs.names);
}

// ==================== DataNameVersionAndReturnCode ===============================================
DataNameVersionAndReturnCode::DataNameVersionAndReturnCode()
    : data_name_and_version(), return_code() {}

DataNameVersionAndReturnCode::DataNameVersionAndReturnCode(
    const DataNameVersionAndReturnCode& other)
    : data_name_and_version(other.data_name_and_version), return_code(other.return_code) {}

DataNameVersionAndReturnCode::DataNameVersionAndReturnCode(DataNameVersionAndReturnCode&& other)
    : data_name_and_version(std::move(other.data_name_and_version)),
      return_code(std::move(other.return_code)) {}

DataNameVersionAndReturnCode& DataNameVersionAndReturnCode::operator=(
    DataNameVersionAndReturnCode other) {
  swap(*this, other);
  return *this;
}

DataNameVersionAndReturnCode::DataNameVersionAndReturnCode(const std::string& serialised_copy)
    : data_name_and_version(), return_code() {
  protobuf::DataNameVersionAndReturnCode proto_copy;
  if (!proto_copy.ParseFromString(serialised_copy))
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));
  data_name_and_version =
      nfs_vault::DataNameAndVersion(proto_copy.serialised_data_name_and_version());
  return_code = ReturnCode(proto_copy.serialised_return_code());
}

std::string DataNameVersionAndReturnCode::Serialise() const {
  protobuf::DataNameVersionAndReturnCode proto_copy;
  proto_copy.set_serialised_data_name_and_version(data_name_and_version.Serialise());
  proto_copy.set_serialised_return_code(return_code.Serialise());
  return proto_copy.SerializeAsString();
}

bool operator==(const DataNameVersionAndReturnCode& lhs, const DataNameVersionAndReturnCode& rhs) {
  return lhs.data_name_and_version == rhs.data_name_and_version &&
         lhs.return_code == rhs.return_code;
}

void swap(DataNameVersionAndReturnCode& lhs, DataNameVersionAndReturnCode& rhs) MAIDSAFE_NOEXCEPT {
  using std::swap;
  swap(lhs.data_name_and_version, rhs.data_name_and_version);
  swap(lhs.return_code, rhs.return_code);
}

// ==================== DataNameOldNewVersionAndReturnCode =========================================
DataNameOldNewVersionAndReturnCode::DataNameOldNewVersionAndReturnCode()
    : data_name_old_new_version(), return_code() {}

DataNameOldNewVersionAndReturnCode::DataNameOldNewVersionAndReturnCode(
    const DataNameOldNewVersionAndReturnCode& other)
    : data_name_old_new_version(other.data_name_old_new_version), return_code(other.return_code) {}

DataNameOldNewVersionAndReturnCode::DataNameOldNewVersionAndReturnCode(
    DataNameOldNewVersionAndReturnCode&& other)
    : data_name_old_new_version(std::move(other.data_name_old_new_version)),
      return_code(std::move(other.return_code)) {}

DataNameOldNewVersionAndReturnCode& DataNameOldNewVersionAndReturnCode::operator=(
    DataNameOldNewVersionAndReturnCode other) {
  swap(*this, other);
  return *this;
}

DataNameOldNewVersionAndReturnCode::DataNameOldNewVersionAndReturnCode(
    const std::string& serialised_copy)
    : data_name_old_new_version(), return_code() {
  protobuf::DataNameOldNewVersionAndReturnCode proto_copy;
  if (!proto_copy.ParseFromString(serialised_copy))
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));
  data_name_old_new_version =
      nfs_vault::DataNameOldNewVersion(proto_copy.serialised_data_name_old_new_version());
  return_code = ReturnCode(proto_copy.serialised_return_code());
}

std::string DataNameOldNewVersionAndReturnCode::Serialise() const {
  protobuf::DataNameOldNewVersionAndReturnCode proto_copy;
  proto_copy.set_serialised_data_name_old_new_version(data_name_old_new_version.Serialise());
  proto_copy.set_serialised_return_code(return_code.Serialise());
  return proto_copy.SerializeAsString();
}

bool operator==(const DataNameOldNewVersionAndReturnCode& lhs,
                const DataNameOldNewVersionAndReturnCode& rhs) {
  return lhs.data_name_old_new_version == rhs.data_name_old_new_version &&
         lhs.return_code == rhs.return_code;
}

void swap(DataNameOldNewVersionAndReturnCode& lhs,
          DataNameOldNewVersionAndReturnCode& rhs) MAIDSAFE_NOEXCEPT {
  using std::swap;
  swap(lhs.data_name_old_new_version, rhs.data_name_old_new_version);
  swap(lhs.return_code, rhs.return_code);
}

// ==================== DataAndReturnCode ==========================================================
DataAndReturnCode::DataAndReturnCode() : data(), return_code() {}

DataAndReturnCode::DataAndReturnCode(const DataAndReturnCode& other)
    : data(other.data), return_code(other.return_code) {}

DataAndReturnCode::DataAndReturnCode(DataAndReturnCode&& other)
    : data(std::move(other.data)), return_code(std::move(other.return_code)) {}

DataAndReturnCode& DataAndReturnCode::operator=(DataAndReturnCode other) {
  swap(*this, other);
  return *this;
}

DataAndReturnCode::DataAndReturnCode(const std::string& serialised_copy) : data(), return_code() {
  protobuf::DataAndReturnCode proto_copy;
  if (!proto_copy.ParseFromString(serialised_copy))
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));
  data = nfs_vault::DataNameAndContent(proto_copy.serialised_data_name_and_content());
  return_code = ReturnCode(proto_copy.serialised_return_code());
}

std::string DataAndReturnCode::Serialise() const {
  protobuf::DataAndReturnCode proto_copy;
  proto_copy.set_serialised_data_name_and_content(data.Serialise());
  proto_copy.set_serialised_return_code(return_code.Serialise());
  return proto_copy.SerializeAsString();
}

bool operator==(const DataAndReturnCode& lhs, const DataAndReturnCode& rhs) {
  return lhs.data == rhs.data && lhs.return_code == rhs.return_code;
}

void swap(DataAndReturnCode& lhs, DataAndReturnCode& rhs) MAIDSAFE_NOEXCEPT {
  using std::swap;
  swap(lhs.data, rhs.data);
  swap(lhs.return_code, rhs.return_code);
}

// ==================== DataNameAndContentOrReturnCode =============================================
template <>
DataNameAndContentOrReturnCode::DataNameAndContentOrReturnCode(
    const DataNameAndReturnCode& data_name_and_return_code)
    : name(data_name_and_return_code.name), return_code(data_name_and_return_code.return_code) {}

DataNameAndContentOrReturnCode::DataNameAndContentOrReturnCode()
    : name(), content(), return_code() {}

DataNameAndContentOrReturnCode::DataNameAndContentOrReturnCode(
    const DataNameAndContentOrReturnCode& other)
        : name(other.name), content(other.content), return_code(other.return_code) {}

DataNameAndContentOrReturnCode::DataNameAndContentOrReturnCode(
    DataNameAndContentOrReturnCode&& other)
        : name(std::move(other.name)), content(std::move(other.content)),
          return_code(std::move(other.return_code)) {}

DataNameAndContentOrReturnCode& DataNameAndContentOrReturnCode::operator=(
    DataNameAndContentOrReturnCode other) {
  swap(*this, other);
  return *this;
}

DataNameAndContentOrReturnCode::DataNameAndContentOrReturnCode(const std::string& serialised_copy)
    : name(), content(), return_code() {
  protobuf::DataNameAndContentOrReturnCode proto_copy;
  if (!proto_copy.ParseFromString(serialised_copy))
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));

  name = nfs_vault::DataName(proto_copy.serialised_name());

  if (proto_copy.has_content())
    content.reset(nfs_vault::Content(proto_copy.content()));
  if (proto_copy.has_serialised_return_code()) {
    return_code.reset(ReturnCode(proto_copy.serialised_return_code()));
  }
  if (!nfs::CheckMutuallyExclusive(content, return_code)) {
    assert(false);
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));
  }
}

std::string DataNameAndContentOrReturnCode::Serialise() const {
  if (!nfs::CheckMutuallyExclusive(content, return_code)) {
    assert(false);
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::serialisation_error));
  }
  protobuf::DataNameAndContentOrReturnCode proto_copy;

  proto_copy.set_serialised_name(name.Serialise());
  if (content)
    proto_copy.set_content(content->Serialise());
  else
    proto_copy.set_serialised_return_code(return_code->Serialise());
  return proto_copy.SerializeAsString();
}

bool operator==(const DataNameAndContentOrReturnCode& lhs,
                const DataNameAndContentOrReturnCode& rhs) {
  if (!(lhs.name == rhs.name))
    return false;

  if (lhs.content)
    return (*lhs.content) == (*rhs.content);
  else if (lhs.return_code)
    return (*lhs.return_code) == (*rhs.return_code);
  return false;
}

void swap(DataNameAndContentOrReturnCode& lhs,
          DataNameAndContentOrReturnCode& rhs) MAIDSAFE_NOEXCEPT {
  using std::swap;
  swap(lhs.name, rhs.name);
  swap(lhs.content, rhs.content);
  swap(lhs.return_code, rhs.return_code);
}

// ==================== StructuredDataNameAndContentOrReturnCode ===================================
StructuredDataNameAndContentOrReturnCode::StructuredDataNameAndContentOrReturnCode()
    : structured_data(), data_name_and_return_code() {}

StructuredDataNameAndContentOrReturnCode::StructuredDataNameAndContentOrReturnCode(
    const StructuredDataNameAndContentOrReturnCode& other)
    : structured_data(other.structured_data),
      data_name_and_return_code(other.data_name_and_return_code) {}

StructuredDataNameAndContentOrReturnCode::StructuredDataNameAndContentOrReturnCode(
    StructuredDataNameAndContentOrReturnCode&& other)
    : structured_data(std::move(other.structured_data)),
      data_name_and_return_code(std::move(other.data_name_and_return_code)) {}

StructuredDataNameAndContentOrReturnCode& StructuredDataNameAndContentOrReturnCode::operator=(
    StructuredDataNameAndContentOrReturnCode other) {
  swap(*this, other);
  return *this;
}

StructuredDataNameAndContentOrReturnCode::StructuredDataNameAndContentOrReturnCode(
    const std::string& serialised_copy)
    : structured_data(), data_name_and_return_code() {
  protobuf::StructuredDataNameAndContentOrReturnCode proto_copy;
  if (!proto_copy.ParseFromString(serialised_copy))
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));

  if (proto_copy.has_serialised_structured_data())
    structured_data.reset(StructuredData(proto_copy.serialised_structured_data()));
  if (proto_copy.has_serialised_data_name_and_return_code()) {
    data_name_and_return_code.reset(
        DataNameAndReturnCode(proto_copy.serialised_data_name_and_return_code()));
  }
  if (!nfs::CheckMutuallyExclusive(structured_data, data_name_and_return_code)) {
    assert(false);
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));
  }
}

std::string StructuredDataNameAndContentOrReturnCode::Serialise() const {
  if (!nfs::CheckMutuallyExclusive(structured_data, data_name_and_return_code)) {
    assert(false);
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::serialisation_error));
  }
  protobuf::StructuredDataNameAndContentOrReturnCode proto_copy;

  if (structured_data)
    proto_copy.set_serialised_structured_data(structured_data->Serialise());
  else
    proto_copy.set_serialised_data_name_and_return_code(data_name_and_return_code->Serialise());
  return proto_copy.SerializeAsString();
}

bool operator==(const StructuredDataNameAndContentOrReturnCode& lhs,
                const StructuredDataNameAndContentOrReturnCode& rhs) {
  if (lhs.structured_data)
    return (*lhs.structured_data) == (*rhs.structured_data);
  else if (lhs.data_name_and_return_code)
    return (*lhs.data_name_and_return_code) == (*rhs.data_name_and_return_code);
  return false;
}

void swap(StructuredDataNameAndContentOrReturnCode& lhs,
          StructuredDataNameAndContentOrReturnCode& rhs) MAIDSAFE_NOEXCEPT {
  using std::swap;
  swap(lhs.structured_data, rhs.structured_data);
  swap(lhs.data_name_and_return_code, rhs.data_name_and_return_code);
}

// ========================== TipOfTreeAndReturnCode ===================================
TipOfTreeAndReturnCode::TipOfTreeAndReturnCode()
    : tip_of_tree(), return_code() {}

TipOfTreeAndReturnCode::TipOfTreeAndReturnCode(const ReturnCode return_code_in)
    : tip_of_tree(), return_code(return_code_in) {}

TipOfTreeAndReturnCode::TipOfTreeAndReturnCode(
    const TipOfTreeAndReturnCode& other)
        : tip_of_tree(other.tip_of_tree), return_code(other.return_code) {}

TipOfTreeAndReturnCode::TipOfTreeAndReturnCode(TipOfTreeAndReturnCode&& other)
        : tip_of_tree(std::move(other.tip_of_tree)), return_code(std::move(other.return_code)) {}

TipOfTreeAndReturnCode& TipOfTreeAndReturnCode::operator=(TipOfTreeAndReturnCode other) {
  swap(*this, other);
  return *this;
}

TipOfTreeAndReturnCode::TipOfTreeAndReturnCode(const std::string& serialised_copy) {
  protobuf::TipOfTreeAndReturnCode proto_copy;
  if (!proto_copy.ParseFromString(serialised_copy))
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));

  if (proto_copy.has_serialised_tip_of_tree()) {
    tip_of_tree.reset(StructuredDataVersions::VersionName(
                          proto_copy.serialised_tip_of_tree()));
  }

  return_code = ReturnCode(proto_copy.serialised_return_code());
}

std::string TipOfTreeAndReturnCode::Serialise() const {
  protobuf::TipOfTreeAndReturnCode proto_copy;

  if (tip_of_tree)
    proto_copy.set_serialised_tip_of_tree(tip_of_tree->Serialise());

  proto_copy.set_serialised_return_code(return_code.Serialise());
  return proto_copy.SerializeAsString();
}

bool operator==(const TipOfTreeAndReturnCode& lhs, const TipOfTreeAndReturnCode& rhs) {
  if ((lhs.tip_of_tree && !rhs.tip_of_tree) || (!lhs.tip_of_tree && rhs.tip_of_tree))
    return false;

  if (lhs.tip_of_tree && (*lhs.tip_of_tree != *rhs.tip_of_tree))
    return false;

  return lhs.return_code == rhs.return_code;
}

void swap(TipOfTreeAndReturnCode& lhs, TipOfTreeAndReturnCode& rhs) MAIDSAFE_NOEXCEPT {
  using std::swap;
  swap(lhs.tip_of_tree, rhs.tip_of_tree);
  swap(lhs.return_code, rhs.return_code);
}

// ==================== DataPmidHintAndReturnCode ==================================================
DataPmidHintAndReturnCode::DataPmidHintAndReturnCode() : data_and_pmid_hint(), return_code() {}

DataPmidHintAndReturnCode::DataPmidHintAndReturnCode(const DataPmidHintAndReturnCode& other)
    : data_and_pmid_hint(other.data_and_pmid_hint), return_code(other.return_code) {}

DataPmidHintAndReturnCode::DataPmidHintAndReturnCode(DataPmidHintAndReturnCode&& other)
    : data_and_pmid_hint(std::move(other.data_and_pmid_hint)),
      return_code(std::move(other.return_code)) {}

DataPmidHintAndReturnCode& DataPmidHintAndReturnCode::operator=(DataPmidHintAndReturnCode other) {
  swap(*this, other);
  return *this;
}

DataPmidHintAndReturnCode::DataPmidHintAndReturnCode(const std::string& serialised_copy)
    : data_and_pmid_hint(), return_code() {
  protobuf::DataPmidHintAndReturnCode proto_copy;
  if (!proto_copy.ParseFromString(serialised_copy))
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));
  data_and_pmid_hint = nfs_vault::DataAndPmidHint(proto_copy.serialised_data_and_pmid_hint());
  return_code = ReturnCode(proto_copy.serialised_return_code());
}

std::string DataPmidHintAndReturnCode::Serialise() const {
  protobuf::DataPmidHintAndReturnCode proto_copy;
  proto_copy.set_serialised_data_and_pmid_hint(data_and_pmid_hint.Serialise());
  proto_copy.set_serialised_return_code(return_code.Serialise());
  return proto_copy.SerializeAsString();
}

bool operator==(const DataPmidHintAndReturnCode& lhs, const DataPmidHintAndReturnCode& rhs) {
  return lhs.data_and_pmid_hint == rhs.data_and_pmid_hint && lhs.return_code == rhs.return_code;
}

void swap(DataPmidHintAndReturnCode& lhs, DataPmidHintAndReturnCode& rhs) MAIDSAFE_NOEXCEPT {
  using std::swap;
  swap(lhs.data_and_pmid_hint, rhs.data_and_pmid_hint);
  swap(lhs.return_code, rhs.return_code);
}

// ==================== PmidRegistrationAndReturnCode ==============================================
PmidRegistrationAndReturnCode::PmidRegistrationAndReturnCode()
    : pmid_registration(), return_code() {}

PmidRegistrationAndReturnCode::PmidRegistrationAndReturnCode(
    nfs_vault::PmidRegistration pmid_health, ReturnCode return_code_in)
        : pmid_registration(std::move(pmid_health)), return_code(std::move(return_code_in)) {}

PmidRegistrationAndReturnCode::PmidRegistrationAndReturnCode(
    const PmidRegistrationAndReturnCode& other)
    : pmid_registration(other.pmid_registration), return_code(other.return_code) {}

PmidRegistrationAndReturnCode::PmidRegistrationAndReturnCode(PmidRegistrationAndReturnCode&& other)
    : pmid_registration(std::move(other.pmid_registration)),
      return_code(std::move(other.return_code)) {}

PmidRegistrationAndReturnCode& PmidRegistrationAndReturnCode::operator=(
    PmidRegistrationAndReturnCode other) {
  swap(*this, other);
  return *this;
}

PmidRegistrationAndReturnCode::PmidRegistrationAndReturnCode(const std::string& serialised_copy)
    : pmid_registration(), return_code() {
  protobuf::PmidRegistrationAndReturnCode proto_copy;
  if (!proto_copy.ParseFromString(serialised_copy))
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));
  pmid_registration = nfs_vault::PmidRegistration(proto_copy.serialised_pmid_registration());
  return_code = ReturnCode(proto_copy.serialised_return_code());
}

std::string PmidRegistrationAndReturnCode::Serialise() const {
  protobuf::PmidRegistrationAndReturnCode proto_copy;
  proto_copy.set_serialised_pmid_registration(pmid_registration.Serialise());
  proto_copy.set_serialised_return_code(return_code.Serialise());
  return proto_copy.SerializeAsString();
}

bool operator==(const PmidRegistrationAndReturnCode& lhs,
                const PmidRegistrationAndReturnCode& rhs) {
  return lhs.pmid_registration == rhs.pmid_registration && lhs.return_code == rhs.return_code;
}

void swap(PmidRegistrationAndReturnCode& lhs,
          PmidRegistrationAndReturnCode& rhs) MAIDSAFE_NOEXCEPT {
  using std::swap;
  swap(lhs.pmid_registration, rhs.pmid_registration);
  swap(lhs.return_code, rhs.return_code);
}

// ==================== DataNameAndSpaceAndReturnCode ==============================================
DataNameAndSpaceAndReturnCode::DataNameAndSpaceAndReturnCode(
    const DataNameAndSpaceAndReturnCode& data)
    : name(data.name), available_space(data.available_space), return_code(data.return_code) {}

DataNameAndSpaceAndReturnCode::DataNameAndSpaceAndReturnCode()
    : name(), available_space(), return_code() {}

DataNameAndSpaceAndReturnCode::DataNameAndSpaceAndReturnCode(const DataTagValue& type_in,
                                                             const Identity& name_in,
                                                             int64_t available_space_in,
                                                             const nfs_client::ReturnCode& code_in)
    : name(nfs_vault::DataName(type_in, name_in)),
      available_space(available_space_in),
      return_code(std::move(code_in)) {}

DataNameAndSpaceAndReturnCode::DataNameAndSpaceAndReturnCode(DataNameAndSpaceAndReturnCode&& other)
    : name(std::move(other.name)),
      available_space(std::move(other.available_space)),
      return_code(std::move(other.return_code)) {}

DataNameAndSpaceAndReturnCode::DataNameAndSpaceAndReturnCode(const std::string& serialised_copy) {
  protobuf::DataNameAndSpaceAndReturnCode proto;
  if (!proto.ParseFromString(serialised_copy))
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));

  name = nfs_vault::DataName(proto.serialised_name());
  available_space = proto.space();
  return_code = nfs_client::ReturnCode(proto.serialised_return_code());
}

std::string DataNameAndSpaceAndReturnCode::Serialise() const {
  protobuf::DataNameAndSpaceAndReturnCode proto_copy;
  proto_copy.set_serialised_name(name.Serialise());
  proto_copy.set_space(available_space);
  proto_copy.set_serialised_return_code(return_code.Serialise());
  return proto_copy.SerializeAsString();
}

DataNameAndSpaceAndReturnCode& DataNameAndSpaceAndReturnCode::operator=(
    DataNameAndSpaceAndReturnCode other) {
  swap(*this, other);
  return *this;
}

void swap(DataNameAndSpaceAndReturnCode& lhs,
          DataNameAndSpaceAndReturnCode& rhs) MAIDSAFE_NOEXCEPT {
  using std::swap;
  swap(lhs.name, rhs.name);
  swap(lhs.available_space, rhs.available_space);
  swap(lhs.return_code, rhs.return_code);
}

bool operator==(const DataNameAndSpaceAndReturnCode& lhs,
                const DataNameAndSpaceAndReturnCode& rhs) {
  return lhs.name == rhs.name && lhs.available_space == rhs.available_space &&
         lhs.return_code == rhs.return_code;
}

// ==================== PmidHealthAndReturnCode ====================================================
PmidHealthAndReturnCode::PmidHealthAndReturnCode(const nfs_vault::PmidHealth& pmid_health_in,
                                                 const nfs_client::ReturnCode& code_in)
      : pmid_health(pmid_health_in), return_code(code_in) {
  LOG(kVerbose) << "PmidHealthAndReturnCode pmid_health.serialised_pmid_health : "
                << HexSubstr(pmid_health.serialised_pmid_health)
                << " pmid_health.Serialise() " << HexSubstr(pmid_health.Serialise())
                << " return_code : " << return_code.value.what();
}

PmidHealthAndReturnCode::PmidHealthAndReturnCode(const std::string& serialised_copy) {
  protobuf::PmidHealthAndReturnCode pmid_health_proto;
  if (!pmid_health_proto.ParseFromString(serialised_copy)) {
    LOG(kError) << "PmidHealthAndReturnCode can't parse from string " << HexSubstr(serialised_copy);
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));
  }
  pmid_health = nfs_vault::PmidHealth(pmid_health_proto.serialised_pmid_health());
  return_code = ReturnCode(pmid_health_proto.serialised_return_code());
  LOG(kVerbose) << "PmidHealthAndReturnCode from string, pmid_health.serialised_pmid_health : "
                << HexSubstr(pmid_health.serialised_pmid_health)
                << " pmid_health.Serialise() " << HexSubstr(pmid_health.Serialise())
                << " return_code : " << return_code.value.what();
}

PmidHealthAndReturnCode::PmidHealthAndReturnCode(const PmidHealthAndReturnCode& other)
    : pmid_health(other.pmid_health),
      return_code(other.return_code) {}

PmidHealthAndReturnCode::PmidHealthAndReturnCode(PmidHealthAndReturnCode&& other)
    : pmid_health(std::move(other.pmid_health)),
      return_code(std::move(other.return_code)) {}

PmidHealthAndReturnCode& PmidHealthAndReturnCode::operator=(PmidHealthAndReturnCode other) {
  swap(*this, other);
  return *this;
}
std::string PmidHealthAndReturnCode::Serialise() const {
  protobuf::PmidHealthAndReturnCode pmid_health_proto;
  pmid_health_proto.set_serialised_pmid_health(pmid_health.Serialise());
  pmid_health_proto.set_serialised_return_code(return_code.Serialise());
  std::string serialised_copy(pmid_health_proto.SerializeAsString());
  LOG(kVerbose) << "PmidHealthAndReturnCode serialised as " << HexSubstr(serialised_copy);
  return serialised_copy;
}

void swap(PmidHealthAndReturnCode& lhs, PmidHealthAndReturnCode& rhs) MAIDSAFE_NOEXCEPT {
  using std::swap;
  swap(lhs.pmid_health, rhs.pmid_health);
  swap(lhs.return_code, rhs.return_code);
}
bool operator==(const PmidHealthAndReturnCode& lhs, const PmidHealthAndReturnCode& rhs) {
  return lhs.pmid_health == rhs.pmid_health &&
         lhs.return_code == rhs.return_code;
}

}  // namespace nfs_client

namespace nfs {

template <>
bool IsSuccess<nfs_client::ReturnCode>(const nfs_client::ReturnCode& response) {
  LOG(kVerbose) << "IsSuccess<nfs_client::ReturnCode> return_code " << response.value.what();
  return response.value.code().value() == static_cast<int>(CommonErrors::success);
}

template <>
std::error_code ErrorCode<nfs_client::ReturnCode>(const nfs_client::ReturnCode& response) {
  return response.value.code();
}

template <>
bool IsSuccess<nfs_client::DataNameAndContentOrReturnCode>(
    const nfs_client::DataNameAndContentOrReturnCode& response) {
  if (response.content) {
    LOG(kVerbose) << "IsSuccess<nfs_client::nfs_client::DataNameAndContentOrReturnCode> "
                  << " data fetched, data_name_and_return_code not initialized";
  } else {
    if (response.return_code) {
      LOG(kWarning)
          << "IsSuccess<nfs_client::nfs_client::DataNameAndContentOrReturnCode> return_code "
          << response.return_code->value.what();
    } else {
      LOG(kError) << "IsSuccess<nfs_client::nfs_client::DataNameAndContentOrReturnCode>"
                  << " neither data or data_name_and_return_code is initialized";
    }
  }

  return response.content;
}

template <>
std::error_code ErrorCode<nfs_client::DataNameAndContentOrReturnCode>(
    const nfs_client::DataNameAndContentOrReturnCode& response) {
  if (response.return_code)
    return response.return_code->value.code();
  else if (response.content)
    return std::error_code(CommonErrors::success);
  else
    return std::error_code(NfsErrors::timed_out);
}

template <>
bool IsSuccess<nfs_client::StructuredDataNameAndContentOrReturnCode>(
    const nfs_client::StructuredDataNameAndContentOrReturnCode& response) {
  if (response.structured_data) {
    LOG(kVerbose) << "IsSuccess<nfs_client::nfs_client::StructuredDataNameAndContentOrReturnCode> "
                  << " structured_data fetched, data_name_and_return_code not initialized";
  } else {
    if (response.data_name_and_return_code)
      LOG(kWarning) << "IsSuccess<nfs_client::nfs_client::StructuredDataNameAndContentOrReturnCode>"
                    << " return_code "
                    << response.data_name_and_return_code->return_code.value.what();
    else
      LOG(kError) << "IsSuccess<nfs_client::nfs_client::StructuredDataNameAndContentOrReturnCode>"
                  << " neither structured_data or data_name_and_return_code is initialized";
  }
  return response.structured_data;
}

template <>
std::error_code ErrorCode<nfs_client::StructuredDataNameAndContentOrReturnCode>(
    const nfs_client::StructuredDataNameAndContentOrReturnCode& response) {
  if (response.data_name_and_return_code)
    return response.data_name_and_return_code->return_code.value.code();
  else if (response.structured_data)
    return std::error_code(CommonErrors::success);
  else
    return std::error_code(NfsErrors::timed_out);
}

}  // namespace nfs

}  // namespace maidsafe
