//
// Created by bale2 on 26/09/2023.
//

#include "sim/progargs.hpp"

#include "gtest/gtest.h"
#include <filesystem>
#include <iostream>
#include <vector>

TEST(TestValidateTimeSteps, Test1) {
  EXPECT_EQ(validate_time_steps(1), 0);
  EXPECT_EQ(validate_time_steps(0), -2);
  EXPECT_EQ(validate_time_steps(-1), -2);
}

TEST(TestValidateInputFile, Test2) {
  std::filesystem::path const currentPath = std::filesystem::current_path();
  std::string const pathfile              = currentPath.string() + "/";
  std::string const testpath              = pathfile + "testfiles/small.fld";
  std::string const nopath                = pathfile + "incorrectpath";
  std::string const corrpath              = pathfile + "smallcorr";
  EXPECT_EQ(validate_input_file(testpath.c_str()), 0);
  EXPECT_EQ(validate_input_file(nopath.c_str()), -1);
  EXPECT_EQ(validate_input_file(corrpath.c_str()), -1);
}

TEST(TestValidateOutputFile, Test3) {
  std::filesystem::path const currentPath = std::filesystem::current_path();
  std::string const pathfile              = currentPath.string() + "/";
  std::string const testpath              = pathfile + "output.fld";
  std::string const nopath;
  EXPECT_EQ(validate_output_file(testpath.c_str()), 0);
  EXPECT_EQ(validate_output_file(nopath.c_str()), -1);
}

TEST(TestValidateParameters, Test4) {
  std::filesystem::path const currentPath = std::filesystem::current_path();
  std::string const pathfile              = currentPath.string() + "/";
  std::array<int, 2> errortype1{};
  std::array<int, 2> errortype2{};
  std::array<int, 2> errortype3{};
  std::array<int, 2> errortype4{};
  std::array<int, 2> errortype5{};
  std::array<int, 2> errortype6{};
  std::array<int, 2> errortype7{};
  std::string const file  = pathfile + "small.fld";
  std::string const file2 = pathfile + "output.fld";
  std::string const file3 = currentPath.string() + "testfiles/largereadonly.fld";
  int const five          = 5;
  errortype1              = {-3, five};
  errortype2              = {-2, -1};
  errortype3              = {-1, 0};
  errortype4              = {0, five};
  errortype5              = {-1, 0};
  errortype6              = {-2, 0};
  errortype7              = {-4, five};

  EXPECT_EQ(validate_parameters(4, {"5", "small.fld", "final.fld", "1000"}), errortype1);
  EXPECT_EQ(validate_parameters(4, {"-1", "small.fld", "final.fld"}), errortype2);
  EXPECT_EQ(validate_parameters(3, {"5", "small.fld", "final.fld"}), errortype3);
  EXPECT_EQ(validate_parameters(4, {"5", pathfile, "final.fld"}), errortype4);
  EXPECT_EQ(validate_parameters(4, {"a", pathfile, "final.fld"}), errortype5);
  EXPECT_EQ(validate_parameters(4, {"0", pathfile, "final.fld"}), errortype6);
  EXPECT_EQ(validate_parameters(4, {"5", pathfile, file3}), errortype7);
}

TEST(TestReadInputFile, Test5) {
  std::filesystem::path const currentPath = std::filesystem::current_path();
  std::string const testpath              = currentPath.string() + "/";
  std::string const file                  = testpath + "testfiles/small.fld";
  Malla const malla                       = read_input_file(file.c_str());
  Malla const mallaux(4800, 204);
  EXPECT_EQ(malla.getNumberParticles(), 4800);
  int const nblocks =
      malla.getNumberBlocks()[0] * malla.getNumberBlocks()[1] * malla.getNumberBlocks()[2];
  EXPECT_EQ(nblocks, 4725);
}

TEST(TestWriteOutputFile, Test6) {
  std::filesystem::path const currentPath = std::filesystem::current_path();
  std::string const testpath              = currentPath.string() + "/";
  std::string const file                  = testpath + "testfiles/small.fld";
  std::string const outfile               = testpath + "testfiles/output.fld";
  Malla malla                             = read_input_file(file.c_str());
  EXPECT_EQ(write_output_file(malla, outfile.c_str()), 0);
}

TEST(TestCheckNP, Test7) {
  EXPECT_NO_THROW(check_number_particles(1));
  EXPECT_THROW(check_number_particles(0), std::runtime_error);
  EXPECT_THROW(check_number_particles(-1), std::runtime_error);
}

TEST(TestCheckMissmatch, Test8) {
  EXPECT_THROW(check_missmatch_particles(0, 1), std::runtime_error);
  EXPECT_THROW(check_missmatch_particles(1, 0), std::runtime_error);
  EXPECT_THROW(check_missmatch_particles(0, -1), std::runtime_error);
  EXPECT_THROW(check_missmatch_particles(-1, 0), std::runtime_error);
  EXPECT_NO_THROW(check_missmatch_particles(0, 0));
}
