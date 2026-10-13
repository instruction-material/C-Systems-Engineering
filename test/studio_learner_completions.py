COMPLETIONS = {
    "CSE-01-systems-build-14": {
        "SENSOR_DECODE": r"""
  const struct SensorFrame decoded = {raw, (uint8_t)(raw / 4096U),
                                      (uint16_t)(raw % 4096U)};
  *out = decoded;
  return FRAME_OK;
""",
        "SENSOR_SCALE": r"""
  const uint32_t scaled = (uint32_t)count * reference_mv;
  const uint32_t quotient = scaled / 4095U, remainder = scaled % 4095U;
  *out = (uint16_t)(quotient + (remainder * 2U >= 4095U ? 1U : 0U));
  return FRAME_OK;
""",
        "SENSOR_BITS": r"""
  char result[17];
  result[16] = '\0';
  uint32_t value = raw;
  for (size_t i = 16; i > 0; --i) {
    result[i - 1U] = (value % 2U) != 0U ? '1' : '0';
    value /= 2U;
  }
  for (size_t i = 0; i < 17; ++i)
    buffer[i] = result[i];
  return FRAME_OK;
""",
    },
    "CSE-02-systems-build-15": {
        "REGION_PLAN": r"""
  struct RegionPlan value = {0};
  value.count = count;
  size_t end = 0;
  for (size_t i = 0; i < count; ++i) {
    const size_t previous = end;
    while (end % specs[i].alignment != 0U) {
      if (end == SIZE_MAX)
        return REGION_OVERFLOW;
      ++end;
    }
    value.padding += end - previous;
    if (specs[i].length > SIZE_MAX - end)
      return REGION_OVERFLOW;
    value.regions[i] =
        (struct RegionPlacement){end, specs[i].length, specs[i].alignment};
    end += specs[i].length;
  }
  if (end > limit)
    return REGION_NO_SPACE;
  value.total = end;
  *out = value;
  return REGION_OK;
""",
        "REGION_LOCATE": r"""
  if (position >= plan->total)
    return REGION_OUTSIDE;
  for (size_t i = 0; i < plan->count; ++i) {
    const struct RegionPlacement *p = &plan->regions[i];
    const size_t end = p->offset + p->length;
    if (position >= p->offset && position < end) {
      *out = (struct RegionAddress){i, position - p->offset};
      return REGION_OK;
    }
  }
  return REGION_PADDING;
""",
    },
    "CSE-03-systems-build-16": {
        "STREAM_FEED": r"""
  if (d->phase == STREAM_FAILED)
    return d->error;
  for (size_t i = 0; i < count; ++i) {
    const uint8_t byte = data[i];
    enum StreamResult failure = STREAM_OK;
    if (d->phase == STREAM_MAGIC) {
      if (byte != 0xA5U)
        failure = STREAM_INVALID;
      else {
        d->crc = crc8_update(0, byte);
        d->phase = STREAM_VERSION;
      }
    } else if (d->phase == STREAM_VERSION) {
      if (byte != 1U)
        failure = STREAM_INVALID;
      else {
        d->crc = crc8_update(d->crc, byte);
        d->phase = STREAM_LENGTH;
      }
    } else if (d->phase == STREAM_LENGTH) {
      if (byte > 32U)
        failure = STREAM_INVALID;
      else {
        d->length = byte;
        d->used = 0;
        d->crc = crc8_update(d->crc, byte);
        d->phase = byte == 0U ? STREAM_CHECKSUM : STREAM_PAYLOAD;
      }
    } else if (d->phase == STREAM_PAYLOAD) {
      d->payload[d->used] = byte;
      ++d->used;
      d->crc = crc8_update(d->crc, byte);
      if (d->used == d->length)
        d->phase = STREAM_CHECKSUM;
    } else if (d->phase == STREAM_CHECKSUM) {
      if (byte != d->crc)
        failure = STREAM_INVALID;
      else if (d->frames == SIZE_MAX)
        failure = STREAM_LIMIT;
      else if (!d->callback(d->payload, d->length, d->context))
        failure = STREAM_CALLBACK_ERROR;
      else {
        d->frames += 1U;
        d->used = 0;
        d->length = 0;
        d->crc = 0;
        d->phase = STREAM_MAGIC;
      }
    } else
      failure = STREAM_INVALID;
    if (failure != STREAM_OK) {
      d->phase = STREAM_FAILED;
      d->error = failure;
      return failure;
    }
  }
  return STREAM_OK;
""",
        "STREAM_FINISH": r"""
  if (d->phase == STREAM_MAGIC)
    return STREAM_OK;
  if (d->phase == STREAM_FAILED)
    return d->error;
  return STREAM_TRUNCATED;
""",
    },
    "CSE-04-systems-build-17": {
        "WINDOW_PARSE": r"""
  size_t comma = length;
  for (size_t i = 0; i < length; ++i)
    if (text[i] == ',') {
      if (comma != length)
        return WINDOW_INVALID;
      comma = i;
    }
  if (comma == length)
    return WINDOW_INVALID;
  uint64_t timestamp = 0, absolute = 0;
  if (!decimal(text, comma, UINT32_MAX, &timestamp))
    return WINDOW_INVALID;
  size_t start = comma + 1U;
  const bool negative = start < length && text[start] == '-';
  if (negative)
    ++start;
  if (!decimal(text + start, length - start, negative ? 40000U : 125000U,
               &absolute))
    return WINDOW_INVALID;
  struct Sample value = {(uint32_t)timestamp, (int32_t)absolute};
  if (negative)
    value.temp_milli_c = -value.temp_milli_c;
  *out = value;
  return WINDOW_OK;
""",
        "WINDOW_AGGREGATE": r"""
  for (size_t i = 0; i < count; ++i) {
    if (samples[i].temp_milli_c < -40000 || samples[i].temp_milli_c > 125000)
      return WINDOW_INVALID;
    if (i > 0U && samples[i - 1U].timestamp_ms > samples[i].timestamp_ms)
      return WINDOW_ORDER;
  }
  struct Bucket temporary[WINDOW_MAX_SAMPLES];
  size_t groups = 0, first = 0;
  while (first < count) {
    const uint32_t key = (samples[first].timestamp_ms / width) * width;
    size_t last = first;
    int64_t total = 0;
    int32_t minimum = 125000, maximum = -40000;
    while (last < count &&
           (samples[last].timestamp_ms / width) * width == key) {
      const int32_t t = samples[last].temp_milli_c;
      total += t;
      if (t < minimum)
        minimum = t;
      if (t > maximum)
        maximum = t;
      ++last;
    }
    const size_t amount = last - first;
    temporary[groups++] = (struct Bucket){
        key, amount, (int32_t)(total / (int64_t)amount), minimum, maximum};
    first = last;
  }
  if (groups > capacity)
    return WINDOW_NO_SPACE;
  for (size_t i = 0; i < groups; ++i)
    out[i] = temporary[i];
  *used = groups;
  return WINDOW_OK;
""",
    },
}
