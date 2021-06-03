
/*
 * Copyright 2017 Google Inc. All rights reserved.
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under
 * the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
 * ANY KIND, either express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

const gulp = require('gulp');
const less = require('gulp-less');
const serve = require('gulp-serve');
const path = require('path');

gulp.task('less-compile', () => {
  return gulp
    .src('./less/**/*.less')
    .pipe(
      less({
        paths: [path.join(__dirname, 'less', 'includes')]
      })
    )
    .pipe(gulp.dest('./css'));
});

gulp.task('js-copy', () => {
    return gulp.src('js/*.js')
      .pipe(gulp.dest('publish/js'));
});

gulp.task('css-copy', () => {
    return gulp.src('css/*.css')
      .pipe(gulp.dest('publish/css'));
});

gulp.task('html-copy', () => {
    return gulp.src('*.html')
      .pipe(gulp.dest('publish'));
});

gulp.task('image-copy', () => {
    return gulp.src('images/*')
      .pipe(gulp.dest('publish/images'));
});

gulp.task(
    'build',
    ['less-compile', 'js-copy', 'css-copy', 'html-copy', 'image-copy']
);

gulp.task(
  'serve',
  ['less-compile'],
  serve({
    root: ['.'],
    port: 8080
  })
);